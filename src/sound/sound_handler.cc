/*
 * Copyright (C) 2005-2009 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "sound/sound_handler.h"

#include <cerrno>
#include <memory>

#include <SDL.h>
#include <SDL_mixer.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#ifdef _WIN32
#include <windows.h>
#endif

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/map.h"
#include "sound/songset.h"
#include "wui/interactive_base.h"
#include "wui/mapview.h"
#include "wui/mapviewpixelfunctions.h"

namespace {

constexpr int kDefaultMusicVolume = 64;
constexpr int kDefaultFxVolume = 128;

void report_initalization_error(const char* msg) {
	log("WARNING: Failed to initialize sound system: %s\n", msg);
	return;
}

}  // namespace

/** The global \ref SoundHandler object
 * The sound handler is a static object because otherwise it'd be quite
 * difficult to pass the --nosound command line option
 */
SoundHandler g_sound_handler;

/** This is just a basic constructor. The \ref SoundHandler must already exist
 * during command line parsing because --nosound needs to be known. At this
 * time, however, all other information is still unknown, so a real
 * initialization cannot take place.
 * \sa SoundHandler::init()
*/
SoundHandler::SoundHandler()
   : egbase_(nullptr),
     nosound_(false),
     lock_audio_disabling_(false),
     disable_music_(false),
     disable_fx_(false),
     music_volume_(MIX_MAX_VOLUME),
     fx_volume_(MIX_MAX_VOLUME),
     random_order_(true),
     current_songset_(""),
     fx_lock_(nullptr) {
}

/// Housekeeping: unset hooks. Audio data will be freed automagically by the
/// \ref Songset and \ref FXset destructors, but not the {song|fx}sets
/// themselves.
SoundHandler::~SoundHandler() {
}

/** The real initialization for SoundHandler.
 *
 * \see SoundHandler::SoundHandler()
*/
void SoundHandler::init() {
	read_config();
	rng_.seed(SDL_GetTicks());
// This RNG will still be somewhat predictable, but it's just to avoid
// identical playback patterns

// Windows Music has crickling inside if the buffer has another size
// than 4k, but other systems work fine with less, some crash
// with big buffers.
#ifdef _WIN32
	const uint16_t bufsize = 4096;
#else
	const uint16_t bufsize = 1024;
#endif

	if (nosound_) {
		set_disable_music(true);
		set_disable_fx(true);
		lock_audio_disabling_ = true;
		return;
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		report_initalization_error(SDL_GetError());
		return;
	}

	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, bufsize) != 0) {
		initialization_error(Mix_GetError());
		return;
	}

	constexpr int kMixInitFlags = MIX_INIT_OGG;
	int initted = Mix_Init(kMixInitFlags);
	if ((initted & kMixInitFlags) != kMixInitFlags) {
		initialization_error("No Ogg support in SDL_Mixer.");
		return;
	}

	Mix_HookMusicFinished(SoundHandler::music_finished_callback);
	Mix_ChannelFinished(SoundHandler::fx_finished_callback);
	load_system_sounds();
	Mix_VolumeMusic(music_volume_);  //  can not do this before InitSubSystem

	if (fx_lock_ == nullptr)
		fx_lock_ = SDL_CreateMutex();
}

void SoundHandler::initialization_error(const std::string& msg) {
	log("WARNING: Failed to initialize sound system: %s\n", msg.c_str());

	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	set_disable_music(true);
	set_disable_fx(true);
	lock_audio_disabling_ = true;
	return;
}

void SoundHandler::shutdown() {
	Mix_ChannelFinished(nullptr);
	Mix_HookMusicFinished(nullptr);

	int numtimesopened, frequency, channels;
	uint16_t format;
	numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
	log("SoundHandler closing times %i, freq %i, format %i, chan %i\n", numtimesopened, frequency,
	    format, channels);

	if (!numtimesopened)
		return;

	Mix_HaltChannel(-1);

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		log("audio error %s\n", SDL_GetError());
	}

	log("SDL_AUDIODRIVER %s\n", SDL_GetCurrentAudioDriver());

	if (numtimesopened != 1) {
		log("PROBLEM: sound device opened multiple times, trying to close");
	}
	for (int i = 0; i < numtimesopened; ++i) {
		Mix_CloseAudio();
	}

	if (fx_lock_) {
		SDL_DestroyMutex(fx_lock_);
		fx_lock_ = nullptr;
	}

	songs_.clear();
	fxs_.clear();

	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

/** Read the main config file, load background music and systemwide sound fx
 *
 */
void SoundHandler::read_config() {
	Section& s = g_options.pull_section("global");

	if (nosound_) {
		set_disable_music(true);
		set_disable_fx(true);
	} else {
		set_disable_music(s.get_bool("disable_music", false));
		set_disable_fx(s.get_bool("disable_fx", false));
		music_volume_ = s.get_int("music_volume", kDefaultMusicVolume);
		fx_volume_ = s.get_int("fx_volume", kDefaultFxVolume);
	}

	random_order_ = s.get_bool("sound_random_order", true);

	register_song("music", "intro");
	register_song("music", "menu");
	register_song("music", "ingame");
}

/** Load systemwide sound fx into memory.
 * \note This loads only systemwide fx. Worker/building fx will be loaded by
 * their respective conf-file parsers
*/
void SoundHandler::load_system_sounds() {
	load_fx_if_needed("sound", "click", "click");
	load_fx_if_needed("sound", "create_construction_site", "create_construction_site");
	load_fx_if_needed("sound", "message", "message");
	load_fx_if_needed("sound/military", "under_attack", "military/under_attack");
	load_fx_if_needed("sound/military", "site_occupied", "military/site_occupied");
	load_fx_if_needed("sound", "lobby_chat", "lobby_chat");
	load_fx_if_needed("sound", "lobby_freshmen", "lobby_freshmen");
}

/** Load a sound effect. One sound effect can consist of several audio files
 * named EFFECT_XX.ogg, where XX is between 00 and 99.
 *
 * Subdirectories of and files under FILENAME_XX can be named anything you want.
 *
 * \param dir        The relative directory where the audio files reside in data/sound
 * \param filename   Name from which filenames will be formed
 *                   (BASENAME_XX.ogg);
 *                   also the name used with \ref play_fx
*/
void SoundHandler::load_fx_if_needed(const std::string& dir,
                                     const std::string& basename,
                                     const std::string& fx_name) {
	assert(g_fs);

	if (!g_fs->is_directory(dir)) {
		throw wexception("SoundHandler: Can't load files from %s, not a directory!", dir.c_str());
	}

	if (nosound_ || fxs_.count(fx_name) > 0)
		return;

	fxs_.insert(std::make_pair(fx_name, std::unique_ptr<FXset>(new FXset())));

	boost::regex re(basename + "_\\d+\\.ogg");
	FilenameSet files = filter(g_fs->list_directory(dir), [&re](const std::string& fn) {
		return boost::regex_match(FileSystem::fs_filename(fn.c_str()), re);
	});

	for (const std::string& path : files) {
		assert(!g_fs->is_directory(path));
		load_one_fx(path, fx_name);
	}
}

/** Add exactly one file to the given fxset.
 * \param path      the effect to be loaded
 * \param fx_name   the fxset to add the file to
 * The file format must be ogg. Otherwise this call will complain and
 * not load the file.
 * \note The complete audio file will be loaded into memory and stays there
 * until the game is finished.
*/
void SoundHandler::load_one_fx(const std::string& path, const std::string& fx_name) {
	if (nosound_)
		return;

	FileRead fr;
	if (!fr.try_open(*g_fs, path)) {
		log("WARNING: Could not open %s for reading!\n", path.c_str());
		return;
	}

	if (Mix_Chunk* const m =
	       Mix_LoadWAV_RW(SDL_RWFromMem(fr.data(fr.get_size(), 0), fr.get_size()), 1)) {
		// Make sure that requested FXset exists

		assert(fxs_.count(fx_name) > 0);

		fxs_[fx_name]->add_fx(m);
	} else
		log("SoundHandler: loading sound effect \"%s\" for FXset \"%s\" "
		    "failed: %s\n",
		    path.c_str(), fx_name.c_str(), Mix_GetError());
}

/** Calculate  the position of an effect in relation to the visible part of the
 * screen.
 * \param position  where the event happened (map coordinates)
 * \return position in widelands' game window: left=0, right=254, not in
 * viewport = -1
 * \note This function can also be used to check whether a logical coordinate is
 * visible at all
*/
int32_t SoundHandler::stereo_position(Widelands::Coords const position_map) {
	if (nosound_)
		return -1;

	assert(egbase_);
	assert(position_map);

	// Viewpoint is the point of the map in pixel which is shown in the upper
	// left corner of window or fullscreen
	const InteractiveBase& ibase = *egbase_->get_ibase();
	Point const vp = ibase.get_viewpoint();

	// Resolution of window or fullscreen
	int32_t const xres = g_gr->get_xres();
	int32_t const yres = g_gr->get_yres();

	// Get pixel coordinates of sound source from map coordinates
	FloatPoint position_pix;
	// NOCOM(#sirver): needs zoom.... but how?
	MapviewPixelFunctions::get_pix(egbase_->map(), position_map, 1., &position_pix);

	// Adjust pixel coordinates to viewpoint
	position_pix.x -= vp.x;
	position_pix.y -= vp.y;
	// Normalizing correct invalid pixel coordinates
	// NOCOM(#sirver): needs zoom
	Point rounded = round(position_pix);
	MapviewPixelFunctions::normalize_pix(egbase_->map(), 1.f, &rounded);

	// Make sure position is inside viewport
	if (rounded.x >= 0 && rounded.x <= xres && rounded.y >= 0 && rounded.y <= yres) {
		return rounded.x * 254 / xres;
	}

	return -1;
}

/** Find out whether to actually play a certain effect right now or rather not
 * (to avoid "sonic overload").
 */
// TODO(unknown): What is the selection algorithm? cf class documentation
bool SoundHandler::play_or_not(const std::string& fx_name,
                               int32_t const stereo_pos,
                               uint8_t const priority) {
	bool allow_multiple = false;  //  convenience for easier code reading
	float evaluation;             // Temporary to calculate single influences
	float probability;            // Weighted total of all influences

	if (nosound_)
		return false;

	// Probability that this fx gets played; initially set according to priority

	//  float division! not integer
	probability = (priority % PRIO_ALLOW_MULTIPLE) / 128.0f;

	// TODO(unknown): what to do with fx that happen offscreen?
	// TODO(unknown): reduce volume? reduce priority? other?
	if (stereo_pos == -1) {
		return false;
	}

	// TODO(unknown): check for a free channel

	if (priority == PRIO_ALWAYS_PLAY) {
		// TODO(unknown): if there is no free channel, kill a running fx and complain
		return true;
	}

	if (priority >= PRIO_ALLOW_MULTIPLE)
		allow_multiple = true;

	// Find out if an fx called fx_name is already running
	bool already_running = false;

	// Access to active_fx_ is protected because it can
	// be accessed from callback
	if (fx_lock_)
		SDL_LockMutex(fx_lock_);

	// starting a block, so I can define a local type for iterating
	{
		for (const auto& fx_pair : active_fx_) {
			if (fx_pair.second == fx_name) {
				already_running = true;
				break;
			}
		}
	}

	if (fx_lock_)
		SDL_UnlockMutex(fx_lock_);

	if (!allow_multiple && already_running)
		return false;

	// TODO(unknown): long time since any play increases weighted_priority
	// TODO(unknown): high general frequency reduces weighted priority
	// TODO(unknown): deal with "coupled" effects like throw_net and retrieve_net

	uint32_t const ticks_since_last_play = SDL_GetTicks() - fxs_[fx_name]->last_used_;

	//  reward an fx for being silent
	if (ticks_since_last_play > SLIDING_WINDOW_SIZE) {
		evaluation = 1;  //  arbitrary value; 0 -> no change, 1 -> probability = 1

		//  "decrease improbability"
		probability = 1 - ((1 - probability) * (1 - evaluation));
	} else {  // Penalize an fx for playing in short succession
		evaluation = static_cast<float>(ticks_since_last_play) / SLIDING_WINDOW_SIZE;
		probability *= evaluation;  //  decrease probability
	}

	// finally: the decision
	// float division! not integer
	return (rng_.rand() % 255) / 255.0f <= probability;
}

/** Play (one of multiple) sound effect(s) with the given name. The effect(s)
 * must have been loaded before with \ref load_fx.
 * \param fx_name  The identifying name of the sound effect, see \ref load_fx .
 * \param map_position  Map coordinates where the event takes place
 * \param priority      How important is it that this FX actually gets played?
 *         (see \ref FXset::priority_)
*/
void SoundHandler::play_fx(const std::string& fx_name,
                           Widelands::Coords const map_position,
                           uint8_t const priority) {
	if (nosound_)
		return;
	play_fx(fx_name, stereo_position(map_position), priority);
}

/** \overload
 * \param fx_name  The identifying name of the sound effect, see \ref load_fx .
 * \param stereo_position  position in widelands' game window, see
 *                         \ref stereo_position
 * \param priority         How important is it that this FX actually gets
 *                         played? (see \ref FXset::priority_)
*/
void SoundHandler::play_fx(const std::string& fx_name,
                           int32_t const stereo_pos,
                           uint8_t const priority) {
	if (nosound_)
		return;

	assert(stereo_pos >= -1);
	assert(stereo_pos <= 254);

	if (get_disable_fx())
		return;

	if (fxs_.count(fx_name) == 0) {
		log("SoundHandler: sound effect \"%s\" does not exist!\n", fx_name.c_str());
		return;
	}

	// See if the FX should be played
	if (!play_or_not(fx_name, stereo_pos, priority))
		return;

	//  retrieve the fx and play it if it's valid
	if (Mix_Chunk* const m = fxs_[fx_name]->get_fx()) {
		const int32_t chan = Mix_PlayChannel(-1, m, 0);
		if (chan == -1)
			log("SoundHandler: Mix_PlayChannel failed\n");
		else {
			Mix_SetPanning(chan, 254 - stereo_pos, stereo_pos);
			Mix_Volume(chan, get_fx_volume());

			// Access to active_fx_ is protected
			// because it can be accessed from callback
			if (fx_lock_)
				SDL_LockMutex(fx_lock_);
			active_fx_[chan] = fx_name;
			if (fx_lock_)
				SDL_UnlockMutex(fx_lock_);
		}
	} else
		log("SoundHandler: sound effect \"%s\" exists but contains no files!\n", fx_name.c_str());
}

/** Load a background song. One "song" can consist of several audio files named
 * FILE_XX.ogg, where XX is between 00 and 99.
 * \param dir        The directory where the audio files reside.
 * \param basename   Name from which filenames will be formed
 *                   (BASENAME_XX.ogg); also the name used with \ref play_fx .
 * This just registers the song, actual loading takes place when
 * \ref Songset::get_song() is called, i.e. when the song is about to be
 * played. The song will automatically be removed from memory when it has
 * finished playing.
*/
void SoundHandler::register_song(const std::string& dir, const std::string& basename) {
	if (nosound_)
		return;
	assert(g_fs);

	FilenameSet files;

	files = filter(g_fs->list_directory(dir), [&basename](const std::string& fn) {
		const std::string only_filename = FileSystem::fs_filename(fn.c_str());
		return boost::starts_with(only_filename, basename) && boost::ends_with(only_filename, ".ogg");
	});

	for (const std::string& filename : files) {
		assert(!g_fs->is_directory(filename));
		if (songs_.count(basename) == 0) {
			songs_.insert(std::make_pair(basename, std::unique_ptr<Songset>(new Songset())));
		}
		songs_[basename]->add_song(filename);
	}
}

/** Start playing a songset.
 * \param songset_name  The songset to play a song from.
 * \param fadein_ms     Song will fade from 0% to 100% during fadein_ms
 *                      milliseconds starting from now.
 * \note When calling start_music() while music is still fading out from
 * \ref stop_music()
 * or \ref change_music() this function will block until the fadeout is complete
*/
void SoundHandler::start_music(const std::string& songset_name, int32_t fadein_ms) {
	if (get_disable_music() || nosound_)
		return;

	if (fadein_ms == 0)
		fadein_ms = 250;  //  avoid clicks

	if (Mix_PlayingMusic())
		change_music(songset_name, 0, fadein_ms);

	if (songs_.count(songset_name) == 0)
		log("SoundHandler: songset \"%s\" does not exist!\n", songset_name.c_str());
	else {
		if (Mix_Music* const m = songs_[songset_name]->get_song()) {
			Mix_FadeInMusic(m, 1, fadein_ms);
			current_songset_ = songset_name;
		} else
			log("SoundHandler: songset \"%s\" exists but contains no files!\n", songset_name.c_str());
	}
	Mix_VolumeMusic(music_volume_);
}

/** Stop playing a songset.
 * \param fadeout_ms Song will fade from 100% to 0% during fadeout_ms
 *                   milliseconds starting from now.
*/
void SoundHandler::stop_music(int32_t fadeout_ms) {
	if (get_disable_music() || nosound_)
		return;

	if (fadeout_ms == 0)
		fadeout_ms = 250;  //  avoid clicks

	Mix_FadeOutMusic(fadeout_ms);
}

/** Play an other piece of music.
 * This is a member function provided for convenience. It is a wrapper around
 * \ref start_music and \ref stop_music.
 * \param fadeout_ms  Old song will fade from 100% to 0% during fadeout_ms
 *                    milliseconds starting from now.
 * \param fadein_ms   New song will fade from 0% to 100% during fadein_ms
 *                    milliseconds starting from now.
 * If songset_name is empty, another song from the currently active songset will
 * be selected
*/
void SoundHandler::change_music(const std::string& songset_name,
                                int32_t const fadeout_ms,
                                int32_t const fadein_ms) {
	if (nosound_)
		return;

	std::string s = songset_name;

	if (s == "")
		s = current_songset_;
	else
		current_songset_ = s;

	if (Mix_PlayingMusic())
		stop_music(fadeout_ms);
	else
		start_music(s, fadein_ms);
}

bool SoundHandler::get_disable_music() const {
	return disable_music_;
}
bool SoundHandler::get_disable_fx() const {
	return disable_fx_;
}
int32_t SoundHandler::get_music_volume() const {
	return music_volume_;
}
int32_t SoundHandler::get_fx_volume() const {
	return fx_volume_;
}

/** Normal set_* function, but the music must be started/stopped accordingly
 * Also, the new value is written back to the config file right away. It might
 * get lost otherwise.
 */
void SoundHandler::set_disable_music(bool const disable) {
	if (lock_audio_disabling_ || disable_music_ == disable)
		return;

	if (disable) {
		stop_music();
		disable_music_ = true;
	} else {
		disable_music_ = false;
		start_music(current_songset_);
	}

	g_options.pull_section("global").set_bool("disable_music", disable);
}

/** Normal set_* function
 * Also, the new value is written back to the config file right away. It might
 * get lost otherwise.
*/
void SoundHandler::set_disable_fx(bool const disable) {
	if (lock_audio_disabling_)
		return;

	disable_fx_ = disable;

	g_options.pull_section("global").set_bool("disable_fx", disable);
}

/**
 * Normal set_* function.
 * Set the music volume between 0 (muted) and \ref get_max_volume().
 * The new value is written back to the config file.
 *
 * \param volume The new music volume.
 */
void SoundHandler::set_music_volume(int32_t volume) {
	if (!lock_audio_disabling_ && !nosound_) {
		music_volume_ = volume;
		Mix_VolumeMusic(volume);
		g_options.pull_section("global").set_int("music_volume", volume);
	}
}

/**
 * Normal set_* function
 * Set the FX sound volume between 0 (muted) and \ref get_max_volume().
 * The new value is written back to the config file.
 *
 * \param volume The new music volume.
 */
void SoundHandler::set_fx_volume(int32_t volume) {
	if (!lock_audio_disabling_ && !nosound_) {
		fx_volume_ = volume;
		Mix_Volume(-1, volume);
		g_options.pull_section("global").set_int("fx_volume", volume);
	}
}

/** Callback to notify \ref SoundHandler that a song has finished playing.
 * Usually, another song from the same songset will be started.
 * There is a special case for the intro screen's music: only one song will be
 * played. If the user has not clicked the mouse or pressed escape when the song
 * finishes, Widelands will automatically go on to the main menu.
*/
void SoundHandler::music_finished_callback() {
	// DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!

	SDL_Event event;
	if (g_sound_handler.current_songset_ == "intro") {
		// Special case for splashscreen: there, only one song is ever played
		event.type = SDL_KEYDOWN;
		event.key.state = SDL_PRESSED;
		event.key.keysym.sym = SDLK_ESCAPE;
	} else {
		// Else just play the next song - see general description for
		// further explanation
		event.type = SDL_USEREVENT;
		event.user.code = CHANGE_MUSIC;
	}
	SDL_PushEvent(&event);
}

/** Callback to notify \ref SoundHandler that a sound effect has finished
 * playing.
*/
void SoundHandler::fx_finished_callback(int32_t const channel) {
	// DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!

	assert(0 <= channel);
	g_sound_handler.handle_channel_finished(static_cast<uint32_t>(channel));
}

/** Remove a finished sound fx from the list of currently playing ones
 * This is part of \ref fx_finished_callback
 */
void SoundHandler::handle_channel_finished(uint32_t channel) {
	// Needs locking because active_fx_ may be accessed
	// from this callback or from main thread
	if (fx_lock_)
		SDL_LockMutex(fx_lock_);
	active_fx_.erase(channel);
	if (fx_lock_)
		SDL_UnlockMutex(fx_lock_);
}
