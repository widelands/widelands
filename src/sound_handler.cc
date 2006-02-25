/*
 * Copyright (C) 2002-2003, 2006 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include "sound_handler.h"

#include <assert.h>
#include <errno.h>
#include "mapviewpixelfunctions.h"
#include "profile.h"
#include "options.h"
#include "error.h"
#include "filesystem.h"
#include "map.h"
#include "interactive_base.h"
#include "mapview.h"

/// Prepare infrastructure for reading song files from disk
Songset::Songset()
{
	m_fr = new FileRead();
	m_rwops = NULL;
	m_m = NULL;
}

/// Close and delete all songs to avoid memory leaks.
Songset::~Songset()
{
	m_songs.clear();

	if (m_m)		//m might be NULL
		free(m_m);

	if (m_rwops)		//rwops might be NULL
		SDL_FreeRW(m_rwops);

	delete m_fr;
}

/** Append a song to the end of the songset
 * \param filename	The song to append
 * \note The \ref current_song will unconditionally be set to the songset's
 * first song. If you do not want to disturb the (linear) playback order then
 * \ref register_song() all songs before you start playing
 */
void Songset::add_song(const string filename)
{
	m_songs.push_back(filename);
	m_current_song = m_songs.begin();
}

/** Get a song from the songset. Depending on
 * \ref Sound_Handler::sound_random_order, the selection will either be random
 * or linear (after last song, will start again with first).
 * \return	a pointer to the chosen song; NULL if none was found, music is
 * 		disabled or an error occurred
 */
Mix_Music *Songset::get_song()
{
	int songnumber;
	string filename;

	if (g_sound_handler.get_disable_music() || m_songs.empty())
		return NULL;

	if (g_sound_handler.m_random_order) {
		songnumber = g_sound_handler.m_rng.rand() % m_songs.size();
		filename = m_songs.at(songnumber);
	} else {
		if (m_current_song == m_songs.end())
			m_current_song = m_songs.begin();

		filename = *(m_current_song++);
	}

	//first, close the previous song and remove it from memory
	if (m_m)		//m might be NULL
		free(m_m);

	if (m_rwops) {		//rwops might be NULL
		SDL_FreeRW(m_rwops);
		m_fr->Close(); //just in case, no-op if already closed
	}

	//then open the new song
	if (m_fr->TryOpen(g_fs, filename))
		m_rwops = SDL_RWFromMem(m_fr->Data(0), m_fr->GetSize());
	else
		return NULL;

	//TODO: review the #ifdef'ed blocks

#ifdef __WIN32__
#warning Mix_LoadMUS_RW is not available under windows!!!
	m_m = Mix_LoadMUS(filename.c_str());	// Hack for windows

#else
#if NEW_SDL_MIXER == 1
	m_m = Mix_LoadMUS_RW(m_rwops);

#else
#warning Please update your SDL_mixer library to at least version 1.2.6!!!
	m_m = Mix_LoadMUS(filename.c_str());

	//TODO: this should use a RWopsified version!
#endif
#endif

	if (m_m) {
		log(("Sound_Handler: loaded song \""+filename+"\"\n").c_str());
	} else {
		log(("Sound_Handler: loading song \"" + filename +
		     "\" failed!\n").c_str());
		log("Sound_Handler: %s\n", Mix_GetError());
	}

	return m_m;
}

//-----------------------------------------------------------------------------

/** Create an FXset and set it's \ref m_priority
 * \param[in] prio	The desired priority (optional)
*/
FXset::FXset(Uint8 prio)
{
	m_priority = prio;
	m_last_used = 0;
}

/// Delete all fxs to avoid memory leaks. This also frees the audio data.
FXset::~FXset()
{
	vector < Mix_Chunk * >::iterator i = m_fxs.begin();

	while (i != m_fxs.end()) {
		Mix_FreeChunk(*i);
		i++;
	}

	m_fxs.clear();
}

/** Append a sound effect to the end of the fxset
 * \param[in] fx	The sound fx to append
 * \param[in] prio	Set previous \ref m_priority to new value (optional)
*/
void FXset::add_fx(Mix_Chunk * fx, Uint8 prio)
{
	assert(fx);

	m_priority = prio;
	m_fxs.push_back(fx);
}

/** Get a sound effect from the fxset. \e Which variant of the fx is actually
 * given out is determined at random
 * \return	a pointer to the chosen effect; NULL if sound effects are
 * disabled or no fx is registered
*/
Mix_Chunk *FXset::get_fx()
{
	int fxnumber;

	if (g_sound_handler.get_disable_fx() || m_fxs.empty())
		return NULL;

	fxnumber = g_sound_handler.m_rng.rand() % m_fxs.size();

	m_last_used = SDL_GetTicks();

	return m_fxs.at(fxnumber);
}

//-----------------------------------------------------------------------------

/** This is just a basic constructor. The \ref Sound_Handler must already exist
 * during command line parsing because --nosound needs to be known. At this
 * time, however, all other information is still unknown, so a real
 * intialization cannot take place.
 * \sa Sound_Handler::init()
*/
Sound_Handler::Sound_Handler()
{
	m_nosound = false;
	m_disable_music = false;
	m_disable_fx = false;
	m_lock_audio_disabling = false;
	m_random_order = true;
	m_current_songset = "";
}

/** Housekeeping: unset hooks. Audio data will be freed automagically by the
 *\ref Songset and \ref FXset destructors
*/
Sound_Handler::~Sound_Handler()
{
	Mix_ChannelFinished(NULL);
	Mix_HookMusicFinished(NULL);

	Mix_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

/** The real intialization for Sound_Handler.
 * \see Sound_Handler::Sound_Handler()
*/
void Sound_Handler::init()
{
	read_config();
	m_rng.seed(SDL_GetTicks());
	//this RNG will still be somewhat predictable, but it's just to avoid
	//identical playback patterns

	//Windows Music has crickling inside if the buffer has another size
	//than 4k, but other systems work fine with less, some crash
	//with big buffers.
#ifdef __WIN32__
	const ushort bufsize = 4096;
#else
	const ushort bufsize = 1024;
#endif

	if ((SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) ||
	      (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,
	                     MIX_DEFAULT_FORMAT, 2,
	                     bufsize) == -1))
	{
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		log("WARNING: Failed to initialize sound system: %s\n",
		    Mix_GetError());

		set_disable_music(true);
		set_disable_fx(true);
		m_lock_audio_disabling=true;
		return;
	} else {
		Mix_HookMusicFinished(Sound_Handler::music_finished_callback);
		Mix_ChannelFinished(Sound_Handler::fx_finished_callback);
		load_system_sounds();
	}
}

/// Read the main config file, load background music and systemwide sound fx
void Sound_Handler::read_config()
{
	Section *s;

	s = g_options.pull_section("global");

	//if there is no config, just ignore future attempts to make us work
	//TODO: really? shouldn't is just be the write-back that gets disabled?
	if (!s) {
		m_nosound = true;
		set_disable_music(true);
		set_disable_fx(true);
		return;
	}

	if (m_nosound) {
		set_disable_music(true);
		set_disable_fx(true);
	} else {
		set_disable_music(s->get_bool("disable_music", false));
		set_disable_fx(s->get_bool("disable_fx", false));
	}

	m_random_order = s->get_bool("sound_random_order", true);

	register_song("music", "intro");
	register_song("music", "menu");
	register_song("music", "ingame");
}

/** Load systemwide sound fx into memory.
 * \note This loads only systemwide fx. Worker/building fx will be loaded by
 * their respective conf-file parsers
*/
void Sound_Handler::load_system_sounds()
{
	load_fx("sound", "click");
	load_fx("sound", "create_construction_site");
}

/** Load a sound effect. One sound effect can consist of several audio files
 * named EFFECT_XX.{ogg, wav}, where XX is between 00 and 99. If
 * BASENAME_XX (without extension) is a directory, effects will be loaded
 * recursively.
 * 
 * Subdirectories of and files under BASENAME_XX can be named anything you want.
 *
 * If you want "internationalized" sound effects (e.g. the lumberjack calling
 * "Timber") then append the locale to any file/directory name like this:
 * lumberjack_timber_00.ogg.de_DE
 * 
 * \param dir        The directory where the audio files reside
 * \param basename	Name from which filenames will be formed
 *                   (BASENAME_XX.wav);
 *                   also the name used with \ref play_fx
 * \internal
 * \param recursive	Whether to recurse into subdirectories
*/
void Sound_Handler::load_fx(const string dir, const string fxname,
                            const bool recursive)
{
	filenameset_t dirs, files;
	filenameset_t::const_iterator i;

	assert(g_fs);

	g_fs->FindFiles(dir, fxname + "_??.???."+Sys_GetLocale(), &files);
	if (files.empty())
		g_fs->FindFiles(dir, fxname + "_??.???", &files);

	for (i = files.begin(); i != files.end(); ++i) {
		assert(!g_fs->IsDirectory(*i));
		load_one_fx(*i, fxname);
	}

	if (recursive) {
		g_fs->FindFiles(dir, "*_??."+Sys_GetLocale(), &dirs);
		if (dirs.empty())
			g_fs->FindFiles(dir, "*_??", &dirs);

		for (i = dirs.begin(); i != dirs.end(); ++i) {
			assert(g_fs->IsDirectory(*i));
			load_fx(*i, fxname, true);
		}
	}
}

/** A wrapper for Mix_LoadWAV that supports RWops
 *
 * In short: always use this instead of Mix_LoadWAV_RW(), because the latter
 * does not work on many systems.
 *
 * For Mix_LoadWAV, there is only preliminary RWops support even in (now
 * current) SDL_mixer-1.2.6 for Linux. In older SDL versions and on other OSes
 * there is no support at all. Widelands' layered filesystem, however, is
 * utterly dependant on RWops. That's the reason for this wrapper. If RWops
 * support is available in Mix_LoadWAV we use it, otherwise we use "normal"
 * RWops (that are available on every platform) to create a tempfile that
 * Mix_LoadWAV can read from.
 *
 * \param fr	Pointer to a FileRead RWops with the
 * \return	a pointer to the loaded sample; NULL if any error ocurred
 * \note This should be phased out when RWops support in Mix_LoadWAV has been
 * available for a sufficiently long time
*/
Mix_Chunk *Sound_Handler::RWopsify_MixLoadWAV(FileRead * fr)
{
	char *tempfile;
	SDL_RWops *target;
	SDL_RWops *src;

	assert(fr);

	//direct access to member variable is neccessary here
	src = SDL_RWFromMem(fr->data, fr->GetSize());

	if (NEW_SDL_MIXER == 1) {
		//SDL_mixer will free the RWops "src" itself
		Mix_Chunk *m = Mix_LoadWAV_RW(src, 1);
		return m;
	} else {
		FILE *f;
		void *buf;

		//create a tempfile
#ifdef __WIN32__
		char szTempName[1024];
		char lpPathBuffer[1024];

		// Get the temp path.
		GetTempPath(1024, lpPathBuffer);
		// Create a temporary file.
		GetTempFileName(lpPathBuffer,	// directory for tmp files
		                "widelands",	// temp file name prefix
		                0,	// create unique name
		                szTempName);	// buffer for name
		tempfile = szTempName;
#else
		//manpage recommends a minimum suffix length of 6
		tempfile = mktemp(strdup("/tmp/widelands-sfx.XXXXXXXX"));
#endif

		if (tempfile == NULL) {
			log("Could not create tempfile "
			    "/tmp/widelands-sfx.XXXXXXXX! Cannot load "
			    "music.");
			return NULL;
		}

		f = fopen(tempfile, "w+");

		if (tempfile == NULL) {
			log("Could not open %s for writing! Cannot load "
			    "music.", tempfile);
			return NULL;
		}

		target = SDL_RWFromFP(f, 0);
		buf = malloc(fr->GetSize());

		if (buf == NULL) {
			log("Could not write to %s! Cannot load music.",
			    tempfile);
			return NULL;
		}

		//write music from src to a tempfile
		SDL_RWread(src, buf, fr->GetSize(), 1);

		SDL_RWwrite(target, buf, fr->GetSize(), 1);

		//load music from tempfile
		Mix_Chunk *m = Mix_LoadWAV(tempfile);

		//remove the RWops on the tempfile
		SDL_RWclose(target);
		SDL_FreeRW(target);

		fclose(f);

		free(buf);

#ifdef __WIN32__
		DeleteFile(tempfile);

#else
		unlink(tempfile);

		free(tempfile);

#endif
		return m;
	}

	SDL_RWclose(src);
	SDL_FreeRW(src);
	return 0;
}

/** Add exactly one file to the given fxset.
 * \param filename	The effect to be loaded
 * \param fx_name	The fxset to add the file to
 * The file format must be ogg or wav. Otherwise this call will complain and
 * not load the file.
 * \note The complete audio file will be loaded into memory and stays there
 * until the game is finished.
*/
void Sound_Handler::load_one_fx(const string filename, const string fx_name)
{
	FileRead fr;
	Mix_Chunk *m;

	if (!fr.TryOpen(g_fs, filename)) {
		log("WARNING: Could not open %s for reading!\n",
		    filename.c_str());
		return;
	}

	m = RWopsify_MixLoadWAV(&fr);

	if (m) {
		//make sure that requested FXset exists

		if (m_fxs.count(fx_name) == 0)
			m_fxs[fx_name] = new FXset();

		m_fxs[fx_name]->add_fx(m);

		log(("Loaded sound effect from \"" + filename +
		     "\" for FXset \"" + fx_name + "\"\n").c_str());
	} else {
		char *msg = (char *) malloc(1024);
		snprintf(msg, 1024, "Sound_Handler: loading sound effect \"%s\""
		         "for FXset \"%s\" failed: %s\n",
		         filename.c_str(), fx_name.c_str(), strerror(errno));
		log(msg);
		free(msg);
	}
}

/** Calculate  the position of an effect in relation to the visible part of the
 * screen.
 * \param position	Where the event happened (map coordinates)
 * \return position in widelands' game window: left=0, right=254, not in
 * viewport = -1
 * \note This function can also be used to check whether a logical coordinate is
 * visible at all
*/
int Sound_Handler::stereo_position(const Coords position)
{
	//screen x,y (without clipping applied, might well be invisible)
	int sx, sy;
	//x,y resolutions of game window
	int xres, yres;
	FCoords fposition;
	Point vp;
	Interactive_Base *ia;

	assert(m_the_game);
	assert(position!=NO_POSITION);
	assert(position!=INVALID_POSITION);

	ia = m_the_game->get_iabase();
	assert(ia);
	vp = ia->get_mapview()->get_viewpoint();

	xres = ia->get_xres();
	yres = ia->get_yres();

	MapviewPixelFunctions::get_pix(*m_the_game->get_map(), position, sx, sy);
	sx -= vp.x;
	sy -= vp.y;

	//make sure position is inside viewport

	if (sx >= 0 && sx <= xres && sy >= 0 && sy <= yres)
		return sx * 254 / xres;

	return -1;
}

/** Find out whether to actually play a certain effect right now or rather not
 * (to avoid "sonic overload").
 * \todo What is the selection algorithm? cf class documentation
*/
bool Sound_Handler::play_or_not(const string fx_name,const int stereo_position,
                                const uint priority)
{
	bool allow_multiple=false; //convenience for easier code reading
	float evaluation; //grade (non-)performance of an fx, can be 0...1
	//this is only an amount, not "good/bad"
	float probability; //used for the final decision

	//probability that this fx gets played; initially set according to priority
	probability=(priority%PRIO_ALLOW_MULTIPLE)/128.0; //float division! not integer

	//TODO: what to do with fx that happen offscreen?
	//TODO: reduce volume? reduce priority? other?
	if (stereo_position == -1) {
		return false;
	}

	//TODO: check for a free channel

	if (priority==PRIO_ALWAYS_PLAY) {
		//TODO: if there is no free channel, kill a running fx and complain
		return true;
	}

	if (priority>=PRIO_ALLOW_MULTIPLE)
		allow_multiple=true;

	//find out if an fx called fx_name is already running
	bool already_running=false;
	map<uint, string>::iterator i;
	for(i=m_active_fx.begin(); i!=m_active_fx.end(); ++i) {
		if ((*i).second==fx_name) {
			already_running=true;
			break;
		}
	}

	if (!allow_multiple && already_running)
		return false;

	//TODO: long time since any play increases weighted_priority
	//TODO: high general frequency reduces weighted priority
	//TODO: deal with "coupled" effects like throw_net and retrieve_net

	uint ticks_since_last_play=SDL_GetTicks()-m_fxs[fx_name]->m_last_used;

	if (ticks_since_last_play>SLIDING_WINDOW_SIZE) { //reward an fx for being silent
		evaluation=1; //arbitrary value; 0->no change, 1->probability=1
		probability=1-((1-probability)*(1-evaluation)); //"decrease improbability"
	} else { //penalize an fx for playing in short succession
		evaluation= static_cast<float>(ticks_since_last_play)/SLIDING_WINDOW_SIZE;
		probability*=evaluation; //decrease probability
	}

	//printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX %s ticks: %i ev: %f prob: %f\n", fx_name.c_str(), ticks_since_last_play, evaluation, probability);

	//finally: the decision
	if ( (m_rng.rand()%255)/255.0 <= probability ) //float division! not integer
		return true;
	else
		return false;
}

/** Play (one of multiple) sound effect(s) with the given name. The effect(s)
 * must have been loaded before with \ref load_fx.
 * \param fx_name	The identifying name of the sound effect, see \ref load_fx
 * \param map_position  Map coordinates where the event takes place
 * \param priority	How important is it that this FX actually gets played?
 *			(see \ref FXset::m_priority)
*/
void Sound_Handler::play_fx(const string fx_name, Coords map_position,
                            const uint priority)
{
	if (map_position == INVALID_POSITION) {
		log("WARNING: play_fx(\"%s\") called without coordinates\n",
		    fx_name.c_str());
		map_position = NO_POSITION;
	}

	if (map_position == NO_POSITION)
		play_fx(fx_name, 128, priority);
	else
		play_fx(fx_name, stereo_position(map_position), priority);
}

/** \overload
 * \param fx_name		The identifying name of the sound effect, see
 * 				\ref load_fx
 * \param stereo_position	position in widelands' game window, see
 * 				\ref stereo_position
 * \param priority		How important is it that this FX actually gets
 * 				played? (see \ref FXset::m_priority)
*/
void Sound_Handler::play_fx(const string fx_name, const int stereo_position,
                            const uint priority)
{
	Mix_Chunk *m;
	int chan;

	assert(stereo_position >= -1 && stereo_position <= 254);

	if (get_disable_fx())
		return;

	if (m_fxs.count(fx_name) == 0) {
		log("Sound_Handler: sound effect \"%s\" does not exist!\n",
		    fx_name.c_str());
		return;
	}

	//see if the FX should be played
	if (!play_or_not(fx_name, stereo_position, priority))
		return;

	//retrieve the fx
	m = m_fxs[fx_name]->get_fx();

	//and play it if it's valid
	if (m) {
		chan = Mix_PlayChannel(-1, m, 0);
		//TODO: complain if this didn't work due to out-of-channels
		Mix_SetPanning(chan,254-stereo_position, stereo_position);
		m_active_fx[chan]=fx_name;
	} else
		log("Sound_Handler: sound effect \"%s\" exists but contains no "
		    "files!\n", fx_name.c_str());
}

/** Load a background song. One "song" can consist of several audio files named
 * FILE_XX.wav (ogg, wav), where XX is between 00 and 99.
 * \param dir		The directory where the audio files reside
 * \param basename	Name from which filenames will be formed
 * 			(BASENAME_XX.wav); also the name used with \ref play_fx
 * \param recursive	\internal Used for traversing subdirectories
 * This just registers the song, actual loading takes place when
 * \ref Songset::get_song() is called, i.e. when the song is about to be
 * played. The song will automatically be removed from memory when it has
 * finished playing.\n
 * Supported file formats are wav and ogg. If BASENAME_XX
 * (with any extension) is a directory, effects will be registered recursively.
 * Subdirectories of and files under BASENAME_XX can be named anything you want.
*/
void Sound_Handler::register_song(const string dir, const string basename,
                                  const bool recursive)
{
	filenameset_t files;
	filenameset_t::const_iterator i;

	assert(g_fs);

	if (recursive)
		g_fs->FindFiles(dir, "*", &files);
	else
		g_fs->FindFiles(dir, basename + "_??*", &files);

	for (i = files.begin(); i != files.end(); ++i) {
		if (g_fs->IsDirectory(*i)) {
			register_song(*i, basename, true);
		} else {
			if (m_songs.count(basename) == 0)
				m_songs[basename] = new Songset();

			m_songs[basename]->add_song(*i);

			log(("Registered song from file \"" + *i +
			     "\" for songset \"" + basename + "\"\n").c_str());
		}
	}
}

/** Start playing a songset.
 * \param songset_name	The songset to play a song from
 * \param fadein_ms	Song will fade from 0% to 100% during fadein_ms
 * 			milliseconds starting from now
 * \note When calling start_music() while music is still fading out from
 * \ref stop_music()
 * or \ref change_music() this function will block until the fadeout is complete
*/
void Sound_Handler::start_music(const string songset_name, int fadein_ms)
{
	if (get_disable_music())
		return;

	if (fadein_ms == 0)
		fadein_ms = 50;	//avoid clicks

	if (Mix_PlayingMusic())
		change_music(songset_name, 0, fadein_ms);

	if (m_songs.count(songset_name) == 0)
		log("Sound_Handler: songset \"%s\" does not exist!\n",
		    songset_name.c_str());
	else {
		Mix_Music *m = m_songs[songset_name]->get_song();

		if (m) {
			Mix_FadeInMusic(m, 1, fadein_ms);
			m_current_songset = songset_name;
		} else
			log("Sound_Handler: songset \"%s\" exists but contains "
			    "no files!\n", songset_name.c_str());
	}

	Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
}

/** Stop playing a songset.
 * \param fadeout_ms Song will fade from 100% to 0% during fadeout_ms milliseconds
 * starting from now.
*/
void Sound_Handler::stop_music(int fadeout_ms)
{
	if (get_disable_music())
		return;

	if (fadeout_ms == 0)
		fadeout_ms = 50;	//avoid clicks

	Mix_FadeOutMusic(fadeout_ms);
}

/** Play an other piece of music.
 * This is a member function provided for convenience. It is a wrapper around
 * \ref start_music and \ref stop_music.
 * \param fadeout_ms	Old song will fade from 100% to 0% during fadeout_ms
 * 			milliseconds starting from now
 * \param fadein_ms	New song will fade from 0% to 100% during fadein_ms
 * 			milliseconds starting from now
 * If songset_name is empty, another song from the currently active songset will
 * be selected
*/
void Sound_Handler::change_music(const string songset_name, int fadeout_ms,
                                 int fadein_ms)
{
	string s;

	s = songset_name;

	if (s == "")
		s = m_current_songset;
	else
		m_current_songset = s;

	if (Mix_PlayingMusic())
		stop_music(fadeout_ms);
	else
		start_music(s, fadein_ms);
}

/// Normal get_* function
bool Sound_Handler::get_disable_music()
{
	return m_disable_music;
}

/// Normal get_* function
bool Sound_Handler::get_disable_fx()
{
	return m_disable_fx;
}

/** Normal set_* function, but the music must be started/stopped accordingly
 * Also, the new value is written back to the config file right away. It might
 * get lost otherwise.
*/
void Sound_Handler::set_disable_music(bool disable)
{
	if (m_lock_audio_disabling)
		return;

	if (disable==true) {
		stop_music();
		m_disable_music = true;
	} else {
		m_disable_music = false;
		start_music(m_current_songset);
	}

	g_options.pull_section("global")->set_bool("disable_music", disable,
	      false);
}

/** Normal set_* function
 * Also, the new value is written back to the config file right away. It might
 * get lost otherwise.
*/
void Sound_Handler::set_disable_fx(bool disable)
{
	if (m_lock_audio_disabling)
		return;

	m_disable_fx = disable;

	g_options.pull_section("global")->set_bool("disable_fx", disable,
	      false);
}

/** Callback to notify \ref Sound_Handler that a song has finished playing.
 * Usually, another song from the same songset will be started.\n
 * There is a special case for the intro screen's music: only one song will be
 * played. If the user has not clicked the mouse or pressed escape when the song
 * finishes, Widelands will automatically go on to the main menu.
*/
void Sound_Handler::music_finished_callback()
{
	//DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!

	//special case for splashscreen: there, only one song is ever played
	if (g_sound_handler.m_current_songset == "intro") {
		SDL_Event *e = new SDL_Event();

		e->type = SDL_KEYDOWN;
		e->key.state = SDL_PRESSED;
		e->key.keysym.sym = SDLK_ESCAPE;
		SDL_PushEvent(e);
	} else {
		//else just play the next song - see general description for
		//further explanation
		SDL_Event *e = new SDL_Event();

		e->type = SDL_USEREVENT;
		e->user.code = SOUND_HANDLER_CHANGE_MUSIC;
		SDL_PushEvent(e);
	}
}

/** Callback to notify \ref Sound_Handler that a sound effect has finished
 * playing.
*/
void Sound_Handler::fx_finished_callback(int channel)
{
	//DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!

	assert(channel>=0);
	g_sound_handler.handle_channel_finished(static_cast<uint>(channel));
}

/** Remove a finished sound fx from the list of currently playing ones
 * This is part of \ref fx_finished_callback
 */
void Sound_Handler::handle_channel_finished(uint channel)
{
	m_active_fx.erase(channel);
}
