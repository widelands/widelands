/*
 * Copyright (C) 2005-2022 by the Widelands Development Team
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

#include <memory>

#include <SDL.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "base/i18n.h"
#include "base/log.h"
#include "wlapplication_options.h"

namespace {
constexpr int kDefaultMusicVolume = 64;
constexpr int kDefaultFxVolume = 128;
constexpr int kNumMixingChannels = 32;
}  // namespace

/// The global \ref SoundHandler object
SoundHandler* g_sh;

bool SoundHandler::backend_is_disabled_ = false;

/**
 * Initialize our data structures, and if SoundHandler::is_backend_disabled() is false, initialize
 * the SDL sound system and configure everything.
 */
SoundHandler::SoundHandler()
   : sound_options_{{SoundType::kUI, SoundOptions(kDefaultFxVolume, "ui")},
                    {SoundType::kMessage, SoundOptions(kDefaultFxVolume, "message")},
                    {SoundType::kChat, SoundOptions(kDefaultFxVolume, "chat")},
                    {SoundType::kAmbient, SoundOptions(kDefaultFxVolume, "ambient")},
                    {SoundType::kMusic, SoundOptions(kDefaultMusicVolume, "music")}},
     fx_lock_(nullptr) {
	// Ensure that we don't lose our config for when we start with sound the next time
	read_config();

	// No sound wanted, let's not do anything.
	if (SoundHandler::is_backend_disabled()) {
		return;
	}

	// This RNG will still be somewhat predictable, but it's just to avoid
	// identical playback patterns
	rng_.seed(SDL_GetTicks());

// Windows Music has crickling inside if the buffer has another size
// than 4k, but other systems work fine with less, some crash
// with big buffers.
#ifdef _WIN32
	const uint16_t bufsize = 4096;
#else
	const uint16_t bufsize = 1024;
#endif

	SDL_version sdl_version;
	SDL_GetVersion(&sdl_version);
	log_info("**** SOUND REPORT ****\n");
	log_info("SDL version: %d.%d.%d\n", static_cast<unsigned int>(sdl_version.major),
	         static_cast<unsigned int>(sdl_version.minor),
	         static_cast<unsigned int>(sdl_version.patch));

	// SDL 2.0.6 will crash due to an upstream bug:
	// https://bugs.launchpad.net/ubuntu/+source/libsdl2/+bug/1722060
	if (sdl_version.major == 2 && sdl_version.minor == 0 && sdl_version.patch == 6) {
		log_warn("Disabled sound due to a bug in SDL 2.0.6\n");
		SoundHandler::disable_backend();
	}

	SDL_MIXER_VERSION(&sdl_version)
	log_info("SDL_mixer version: %d.%d.%d\n", static_cast<unsigned int>(sdl_version.major),
	         static_cast<unsigned int>(sdl_version.minor),
	         static_cast<unsigned int>(sdl_version.patch));

	log_info("**** END SOUND REPORT ****\n");

	if (SoundHandler::is_backend_disabled()) {
		return;
	}

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		initialization_error(SDL_GetError(), false);
		return;
	}

	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, bufsize) != 0) {
		initialization_error(Mix_GetError(), true);
		return;
	}

	constexpr int kMixInitFlags = MIX_INIT_OGG;
	int init_flags = Mix_Init(kMixInitFlags);
	if ((init_flags & kMixInitFlags) != kMixInitFlags) {
		initialization_error("No Ogg support in SDL_Mixer.", true);
		return;
	}

	if (Mix_AllocateChannels(kNumMixingChannels) != kNumMixingChannels) {
		initialization_error(Mix_GetError(), true);
		return;
	}

	Mix_HookMusicFinished(SoundHandler::music_finished_callback);
	Mix_ChannelFinished(SoundHandler::fx_finished_callback);
	Mix_VolumeMusic(sound_options_.at(SoundType::kMusic).volume);

	if (fx_lock_ == nullptr) {
		fx_lock_ = SDL_CreateMutex();
	}
}

/**
 * Housekeeping: unset hooks, clear the mutex and all data structures and shut down the sound
 * system. Audio data will be freed automagically by the \ref Songset and \ref FXset destructors,
 * but not the {song|fx}sets themselves.
 */
SoundHandler::~SoundHandler() {
	if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
		return;
	}

	Mix_ChannelFinished(nullptr);
	Mix_HookMusicFinished(nullptr);
	stop_music();
	songs_.clear();
	fxs_.clear();

	int numtimesopened, frequency, channels;
	uint16_t format;
	numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
	log_info("SoundHandler: Closing %i time%s, %i Hz, format %i, %i channel%s\n", numtimesopened,
	         numtimesopened == 1 ? "" : "s", frequency, format, channels, channels == 1 ? "" : "s");

	if (numtimesopened == 0) {
		return;
	}

	Mix_HaltChannel(-1);

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		log_err("SoundHandler: Audio error %s\n", SDL_GetError());
	}

	log_info("SoundHandler: SDL_AUDIODRIVER %s\n", SDL_GetCurrentAudioDriver());

	if (numtimesopened != 1) {
		log_warn("SoundHandler: PROBLEM: sound device opened multiple times, trying to close");
	}
	for (int i = 0; i < numtimesopened; ++i) {
		Mix_CloseAudio();
	}

	if (fx_lock_) {
		SDL_DestroyMutex(fx_lock_);
		fx_lock_ = nullptr;
	}

	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

/// Prints an error and disables and shuts down the sound system.
void SoundHandler::initialization_error(const char* const msg, bool quit_sdl) {
	log_warn("Failed to initialize sound system: %s\n", msg);
	SoundHandler::disable_backend();
	if (quit_sdl) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
}

/**
 * Load the sound options from cached config. If an option is not available, use the defaults set by
 * the constructor.
 */
void SoundHandler::read_config() {
	for (auto& option : sound_options_) {
		option.second.volume =
		   get_config_int("sound", "volume_" + option.second.name, option.second.volume);
		option.second.enabled =
		   get_config_bool("sound", "enable_" + option.second.name, option.second.enabled);
	}
	use_custom_songset_instead_ingame_ = get_config_bool("sound", "custom_ingame_music", false);
}

/// Save the current sound options to config cache
void SoundHandler::save_config() {
	for (auto& option : sound_options_) {
		const int volume = option.second.volume;
		const std::string& name = option.second.name;
		const bool enabled = option.second.enabled;

		const std::string enable_name = "enable_" + name;
		set_config_bool("sound", enable_name, enabled);

		const std::string volume_name = "volume_" + name;
		set_config_int("sound", volume_name, volume);
	}
	set_config_bool("sound", "custom_ingame_music", use_custom_songset_instead_ingame_);
}

/// Read the sound options from the cache and apply them
void SoundHandler::load_config() {
	read_config();
	for (auto& option : sound_options_) {
		set_volume(option.first, option.second.volume);
		set_enable_sound(option.first, option.second.enabled);
	}
}

/** Register a sound effect. One sound effect can consist of several audio files
 * named EFFECT_XX.ogg, where XX is between 00 and 99.
 *
 * Subdirectories of and files under FILENAME_XX can be named anything you want.
 *
 * \param type       The category of the FxSet to create
 * \param fx_path    The relative path and base filename from which filenames will be formed
 *                   (<datadir>/fx_path_XX.ogg). If an effect with the same 'type' and 'fx_path'
 * already exists, we assume that it is already registered and skip it. \returns  An ID for the
 * effect that can be used to identify it in \ref play_fx.
 */

FxId SoundHandler::register_fx(SoundType type, const std::string& fx_path) {
	if (SoundHandler::is_backend_disabled() || g_sh == nullptr) {
		return kNoSoundEffect;
	}
	return g_sh->do_register_fx(type, fx_path);
}

/// Non-static implementation of register_fx
FxId SoundHandler::do_register_fx(SoundType type, const std::string& fx_path) {
	assert(!SoundHandler::is_backend_disabled());
	if (fx_ids_[type].count(fx_path) == 0) {
		const FxId new_id = fxs_[type].size();
		fx_ids_[type].insert(std::make_pair(fx_path, new_id));
		fxs_[type].insert(
		   std::make_pair(new_id, std::unique_ptr<FXset>(new FXset(fx_path, rng_.rand()))));
		return new_id;
	} else {
		return fx_ids_[type].at(fx_path);
	}
}

/**
 * Find out whether to actually play a certain effect right now or rather not
 * (to avoid "sonic overload"). Based on priority and on when it was last played.
 * System sounds and sounds with priority "kFxPriorityAlwaysPlay" always return 'true'.
 */
bool SoundHandler::play_or_not(SoundType type,
                               const FxId fx_id,
                               uint16_t const priority,
                               bool allow_multiple) {
	assert(!SoundHandler::is_backend_disabled() && is_sound_enabled(type));
	assert(priority >= kFxPriorityLowest);

	if (fxs_[type].count(fx_id) == 0) {
		return false;
	}

	if (type != SoundType::kAmbient) {
		// We always play UI, chat and system sounds
		return true;
	}

	// We always play important sounds
	if (priority == kFxMaximumPriority && allow_multiple) {
		return true;
	}

	// Do not run multiple instances of the same sound effect if the priority is too low
	if (!allow_multiple) {
		// Find out if an fx called 'fx_name' is already running
		lock_fx();
		for (const auto& fx_pair : active_fx_) {
			if (fx_pair.second == fx_id) {
				release_fx_lock();
				return false;
			}
		}
		release_fx_lock();
		// We always play sounds with 100% chance at least once
		if (priority == kFxMaximumPriority) {
			return true;
		}
	}

	// TODO(unknown): long time since any play increases weighted_priority
	// TODO(unknown): high general frequency reduces weighted priority
	// TODO(unknown): deal with "coupled" effects like throw_net and retrieve_net

	uint32_t const ticks_since_last_play = fxs_[type][fx_id]->ticks_since_last_play();

	// Weighted total probability that this fx gets played; initially set according to priority
	//  float division! not integer
	float probability = static_cast<float>(priority) / kFxMaximumPriority;

	// How many milliseconds in the past to consider
	constexpr uint32_t kSlidingWindowSize = 20000;

	if (ticks_since_last_play > kSlidingWindowSize) {  //  reward an fx for being silent
		const float evaluation = 1.0f;  //  arbitrary value; 0 -> no change, 1 -> probability = 1

		//  "decrease improbability"
		probability = 1.0f - ((1.0f - probability) * (1.0f - evaluation));
	} else {  // Penalize an fx for playing in short succession
		const float evaluation = static_cast<float>(ticks_since_last_play) / kSlidingWindowSize;
		probability *= evaluation;  //  decrease probability
	}

	// finally: the decision
	// float division! not integer
	return (rng_.rand() % kFxMaximumPriority) / static_cast<float>(kFxMaximumPriority) <=
	       probability;
}

/**
 * \param type             The categorization of the sound effect to be played
 * \param fx_id            The ID of the sound effect, see \ref register_fx
 * \param priority         How important is it that this FX actually gets
 *                         played? (see \ref FXset::priority_)
 * \param stereo_position  Position in widelands' game window
 * \param distance         Distance in widelands' game window
 */
void SoundHandler::play_fx(SoundType type,
                           const FxId fx_id,
                           uint16_t const priority,
                           bool allow_multiple,
                           int32_t const stereo_pos,
                           int distance) {
	if (SoundHandler::is_backend_disabled() || !is_sound_enabled(type)) {
		return;
	}

	assert(stereo_pos >= kStereoLeft);
	assert(stereo_pos <= kStereoRight);

	if (fx_id == kNoSoundEffect) {
		throw wexception("SoundHandler: Trying to play sound effect that was never registered. Maybe "
		                 "you registered it before instantiating g_sh?\n");
	}

	if (fxs_[type].count(fx_id) == 0) {
		log_err("SoundHandler: Sound effect %d does not exist!\n", fx_id);
		return;
	}

	// See if the FX should be played
	if (!play_or_not(type, fx_id, priority, allow_multiple)) {
		return;
	}

	//  retrieve the fx and play it if it's valid
	if (Mix_Chunk* const m = fxs_[type][fx_id]->get_fx(rng_.rand())) {
		const int32_t chan = Mix_PlayChannel(-1, m, 0);
		if (chan == -1) {
			log_err("SoundHandler: Mix_PlayChannel failed: %s\n", Mix_GetError());
		} else {
			Mix_SetPanning(chan, kStereoRight - stereo_pos, stereo_pos);
			Mix_SetDistance(chan, distance);
			Mix_Volume(chan, get_volume(type));

			lock_fx();
			active_fx_[chan] = fx_id;
			release_fx_lock();
		}
	} else {
		log_err("SoundHandler: Sound effect %d exists but contains no files!\n", fx_id);
	}
}

void SoundHandler::load_fx(SoundType type, FxId id) {
	fxs_[type][id]->load_sound_files();
}

/// Removes the given FXset from memory
void SoundHandler::remove_fx_set(SoundType type) {
	fxs_.erase(type);
	fx_ids_.erase(type);
}

/**
 * Register a background songset. A songset can consist of several audio files named
 * FILE_XX.ogg, where XX is between 00 and 99.
 * \param dir        The directory where the audio files reside.
 * \param basename   Name from which filenames will be formed
 *                   (BASENAME_XX.ogg); also the name used with \ref change_music .
 * This just registers the songs, actual loading takes place when
 * \ref Songset::get_song() is called, i.e. when a song is about to be
 * played. The song will automatically be removed from memory when it has
 * finished playing.
 */
void SoundHandler::register_songs(const std::string& dir, const std::string& basename) {
	if (SoundHandler::is_backend_disabled()) {
		return;
	}
	if (songs_.count(basename) == 0) {
		songs_.insert(std::make_pair(basename, std::unique_ptr<Songset>(new Songset(dir, basename))));
	}
}

/**
 * Start playing a songset.
 * \param songset_name  The songset to play a song from.
 * \note When calling start_music() while music is still fading out from \ref stop_music() or \ref
 * change_music(), this function will block until the fadeout is complete
 */
void SoundHandler::start_music(const std::string& songset_name) {
	if (SoundHandler::is_backend_disabled() || !is_sound_enabled(SoundType::kMusic)) {
		return;
	}

	if (Mix_PlayingMusic()) {
		change_music(songset_name, kMinimumMusicFade);
	}

	if (songs_.count(songset_name) == 0) {
		log_err("SoundHandler: songset \"%s\" does not exist!\n", songset_name.c_str());
	} else {
		if (Mix_Music* const m = songs_[songset_name]->get_song(rng_.rand())) {
			Mix_FadeInMusic(m, 1, kMinimumMusicFade);
			current_songset_ = songset_name;
		} else {
			log_err(
			   "SoundHandler: songset \"%s\" exists but contains no files!\n", songset_name.c_str());
		}
	}
}

/**
 * Stop playing a songset.
 * \param fadeout_ms Song will fade from 100% to 0% during fadeout_ms
 *                   milliseconds starting from now.
 */
void SoundHandler::stop_music(int fadeout_ms) {
	if (SoundHandler::is_backend_disabled()) {
		return;
	}

	if (Mix_PlayingMusic()) {
		Mix_FadeOutMusic(std::max(fadeout_ms, kMinimumMusicFade));
	}
}

/**
 * Play a new piece of music.
 * This is a member function provided for convenience. It is a wrapper around
 * \ref start_music and \ref stop_music.
 * \param fadeout_ms  Old song will fade from 100% to 0% during fadeout_ms
 *                    milliseconds starting from now.
 * If songset_name is empty, another song from the currently active songset will
 * be selected
 */
void SoundHandler::change_music(const std::string& songset_name, int const fadeout_ms) {
	if (SoundHandler::is_backend_disabled()) {
		return;
	}

	if (!songset_name.empty()) {
		if (songset_name == Songset::kIngame && use_custom_songset_instead_ingame_) {
			current_songset_ = Songset::kCustom;
		} else {
			current_songset_ = songset_name;
		}
	}

	if (Mix_PlayingMusic()) {
		stop_music(fadeout_ms);
	} else {
		start_music(current_songset_);
	}
}
bool SoundHandler::use_custom_songset() const {
	return use_custom_songset_instead_ingame_;
}

void SoundHandler::use_custom_songset(bool on) {
	use_custom_songset_instead_ingame_ = on;
	if (current_songset_ == Songset::kIngame && on) {
		change_music(Songset::kCustom);
	} else if (current_songset_ == Songset::kCustom && !on) {
		change_music(Songset::kIngame);
	}
}

/// Returns the currently playing songset
const std::string SoundHandler::current_songset() const {
	return current_songset_;
}

/// Returns whether we want to hear sounds of the given 'type'
bool SoundHandler::is_sound_enabled(SoundType type) const {
	assert(sound_options_.count(type) == 1);
	return sound_options_.at(type).enabled;
}

/// Returns the volume that the given 'type' of sound is to be played at
int32_t SoundHandler::get_volume(SoundType type) const {
	assert(sound_options_.count(type) == 1);
	return sound_options_.at(type).volume;
}

/**
 * Sets that we want to / don't want to hear the given 'type' of sounds. If the type is \ref
 * SoundType::kMusic, start/stop the music as well.
 */
void SoundHandler::set_enable_sound(SoundType type, bool const enable) {
	if (SoundHandler::is_backend_disabled()) {
		return;
	}
	assert(sound_options_.count(type) == 1);

	SoundOptions& sound_options = sound_options_.at(type);
	sound_options.enabled = enable;

	// Special treatment for music
	switch (type) {
	case SoundType::kMusic:
		if (enable) {
			if (!Mix_PlayingMusic()) {
				start_music(current_songset_);
			}
		} else {
			stop_music();
		}
		break;
	default:
		break;
	}
}

/**
 * Sets the music or sound 'volume' for the given 'type' between 0 (muted) and \ref
 * get_max_volume().
 */
void SoundHandler::set_volume(SoundType type, int32_t volume) {
	if (SoundHandler::is_backend_disabled()) {
		return;
	}

	assert(sound_options_.count(type) == 1);
	assert(volume >= 0 && volume <= get_max_volume());

	sound_options_.at(type).volume = volume;

	// Special treatment for music
	switch (type) {
	case SoundType::kMusic:
		Mix_VolumeMusic(volume);
		break;
	default:
		Mix_Volume(-1, volume);
		break;
	}
}

/**
 * Returns the max value for volume settings. We use a function to hide
 * SDL_mixer constants outside of sound_handler.
 */
int32_t SoundHandler::get_max_volume() const {
	return MIX_MAX_VOLUME;
}

/**
 * Callback to notify \ref SoundHandler that a song has finished playing.
 * Pushes an SDL_Event with type = SDL_USEREVENT and user.code = CHANGE_MUSIC.
 */
void SoundHandler::music_finished_callback() {
	// DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!

	assert(!SoundHandler::is_backend_disabled());
	// Trigger that we want a music change and leave the specifics to the application.
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = CHANGE_MUSIC;
	SDL_PushEvent(&event);
}

/**
 * Callback to notify \ref SoundHandler that a sound effect has finished
 * playing. Removes the finished sound fx from the list of currently playing ones.
 */
void SoundHandler::fx_finished_callback(int32_t const channel) {
	// DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!

	assert(!SoundHandler::is_backend_disabled());
	assert(0 <= channel);
	g_sh->lock_fx();
	g_sh->active_fx_.erase(static_cast<uint32_t>(channel));
	g_sh->release_fx_lock();
}

/// Lock the SDL mutex. Access to 'active_fx_' is protected by mutex because it can be accessed both
/// from callbacks or from the main thread.
void SoundHandler::lock_fx() {
	if (fx_lock_) {
		SDL_LockMutex(fx_lock_);
	}
}

/// Release the SDL mutex
void SoundHandler::release_fx_lock() {
	if (fx_lock_) {
		SDL_UnlockMutex(fx_lock_);
	}
}
