/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include "sound_handler.h"

#include <assert.h>
#include <errno.h>
#include "profile.h"
#include "options.h"
#include "error.h"
#include "filesystem.h"
#include "map.h"
#include "interactive_base.h"
#include "mapview.h"

/** \todo: Error checking, error checking, error checking */

/** Prepare infrastructure for reading song files from disk*/
Songset::Songset()
{
	fr = new FileRead();
	rwops = NULL;
	m = NULL;
}

/** Close and delete all songs to avoid memory leaks.*/
Songset::~Songset()
{
	songs.clear();

	if (m)			//m might be NULL
		free(m);
	if (rwops)		//rwops might be NULL
		SDL_FreeRW(rwops);
	delete fr;
}

/** Append a song to the end of the songset
 * \param filename	The song to append
 * \note The \ref current_song will unconditionally be set to the songset's first song.
 * If you do not want to disturb the (linear) playback order then \ref register_song all
 * songs before you start playing
 */
void Songset::add_song(string filename)
{
	songs.push_back(filename);
	current_song = songs.begin();
}

/** Get a song from the songset. Depending on \ref Sound_Handler::sound_random_order, 
 * the selection will either be random or linear (after last song, will 
 * start again with first.*/
Mix_Music *Songset::get_song()
{
	int songnumber;
	string filename;

	if (g_sound_handler.disable_music || songs.empty())
		return NULL;

	if (g_sound_handler.random_order) {
		songnumber = g_sound_handler.rng.rand() % songs.size();
		filename = songs.at(songnumber);
	} else {
		if (current_song == songs.end())
			current_song = songs.begin();
		filename = *(current_song++);
	}

	//first, close the previous song and remove it from memory
	if (m)			//m might be NULL
		free(m);
	if (rwops) {		//rwops might be NULL
		SDL_FreeRW(rwops);
		fr->Close();
	}
	//then open the new song
	fr->Open(g_fs, filename);
	rwops = SDL_RWFromMem(fr->Data(0), fr->GetSize());

#ifdef __WIN32__
#warning Mix_LoadMUS_RW is not available under windows!!!
	m = NULL;
#else
#ifdef OLD_SDL_MIXER
#warning Please update your SDL_mixer library to at least version 1.2.6!!!
	m = Mix_LoadMUS(filename.c_str());
#else
	m = Mix_LoadMUS_RW(rwops);
#endif
#endif

	if (m) {
		log(("Sound_Handler: loaded song \"" + filename + "\"\n").c_str());
	} else {
		log(("Sound_Handler: loading song \"" + filename + "\" failed!\n").c_str());
		log("Sound_Handler: %s\n", Mix_GetError());
	}

	return m;
}

//--------------------------------------------------------------------------------------

/** Create an FXset and set it's \ref priority
 * \param[in] prio	The desired priority (optional)*/
FXset::FXset(Uint8 prio)
{
	priority = prio;
	last_used = 0;
}

/** Delete all fxs to avoid memory leaks. This also frees the audio data.*/
FXset::~FXset()
{
	fxs.clear();
}

/** Append a sound effect to the end of the fxset
 * \param[in] fx	The sound fx to append
 * \param[in] prio	Set previous \ref priority to new value (optional)*/
void FXset::add_fx(Mix_Chunk * fx, Uint8 prio)
{
	assert(fx);

	priority = prio;
	fxs.push_back(fx);
}

/** Get a sound effect from the fxset. \e Which variant of the fx is actually given
 * out is determined at random
 * \todo Implement priorities
*/
Mix_Chunk *FXset::get_fx()
{
	int fxnumber;

	if (g_sound_handler.disable_fx || fxs.empty())
		return NULL;

	fxnumber = g_sound_handler.rng.rand() % fxs.size();
	last_used = SDL_GetTicks();

	return fxs.at(fxnumber);
}

//-----------------------------------------------------------------------------------------

/** This is just a basic constructor. The \ref Sound_Handler must already exist during command line parsing
 * because --nosound needs to be known. At this time, however, all other information is still unknown,
 * so a real intialization cannot take place.
 * \see Sound_Handler::init()
*/
Sound_Handler::Sound_Handler()
{
	nosound = false;
	disable_music = false;
	disable_fx = false;
	random_order = true;
	current_songset = "";
}

/** Housekeeping: unset hooks. Audio data will be freed automagically by the \ref Songset and \ref FXset destructors*/
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
	rng.seed(SDL_GetTicks());	//that's still somewhat predictable, but it's just to avoid identical
	//playback patterns

	SDL_InitSubSystem(SDL_INIT_AUDIO);
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) == -1) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		log("WARNING: Failed to initialize sound system: %s\n", Mix_GetError());
		disable_music = true;
		disable_fx = true;
		return;
	}

	Mix_HookMusicFinished(Sound_Handler::music_finished_callback);
	Mix_ChannelFinished(Sound_Handler::fx_finished_callback);

	load_system_sounds();
}

/** Read the main config file, load background music and systemwide sound effects*/
void Sound_Handler::read_config()
{
	Section *s;

	s = g_options.pull_section("global");

	if (nosound) {
		disable_music = true;
		disable_fx = true;
	} else {
		disable_music = s->get_bool("disable_music", false);
		disable_fx = s->get_bool("disable_fx", false);
	}
	random_order = s->get_bool("sound_random_order", true);

	register_song("music", "intro");
	register_song("music", "menu");
	register_song("music", "ingame");
}

/** Load systemwide sound fx into memory (per_object fx will be loaded by the object)
 * \note This loads only systemwide fx. Worker/building fx will be loaded by their
 * respective conf-file parsers
*/
void Sound_Handler::load_system_sounds()
{
	load_fx("sound", "create_construction_site");
}

/** Load a sound effect. One sound effect can consist of several audio files named
 * EFFECT_XX.wav (ogg, wav), where XX is between 00 and 99.
 * If BASENAME_XX (with any or no extension) is a directory, effects will be loaded recursively.
 * Subdirectories of and files under BASENAME_XX can be named anything you want.
 * \param dir		The directory where the audio files reside
 * \param basename	Name from which filenames will be formed (BASENAME_XX.wav);
 * 			also the name used with \ref play_fx
 * \internal
 * \param recursive	Whether to recurse into subdirectories
 */
void Sound_Handler::load_fx(const string dir, const string basename, const bool recursive)
{
	filenameset_t files;
	filenameset_t::const_iterator i;

	if (recursive)
		g_fs->FindFiles(dir, "*", &files);
	else
		g_fs->FindFiles(dir, basename + "_??.*", &files);

	for (i = files.begin(); i != files.end(); ++i) {
		if (g_fs->IsDirectory(*i)) {
			load_fx(*i, basename, true);
		} else
			load_one_fx(*i, basename);
	}
}

/** A wrapper for Mix_LoadWAV that supports RWops
  * \param fr	Pointer to a FileRead RWops with the
  * For Mix_LoadWAV, there is only preliminary RWops support even in (now current) SDL-1.2.6 for Linux.
  * In older SDL versions and on other OSes there is no support at all. Widelands' layered filesystem, however,
  * is utterly dependant on RWops. That's the reason for this wrapper. If RWops support is available in Mix_LoadWAV
  * we use it, otherwise we use "normal" RWops (that are available on every platform) to create a tempfile
  * that Mix_LoadWAV can read from.
  * \note This should be phased out when RWops support in Mix_LoadWAV has been available for a sufficiently long
  * time
*/
Mix_Chunk *Sound_Handler::RWopsify_MixLoadWAV(FileRead * fr)
{
	string tempfile;
	SDL_RWops *target;
	SDL_RWops *src;

	assert(fr);

	src = SDL_RWFromMem(fr->data, fr->GetSize());	//direct access to member variable is neccessary here

	if (USE_RWOPS) {
		Mix_Chunk *m = Mix_LoadWAV_RW(src, 1);	//SDL_mixer will free the RWops "src" itself
		return m;
	} else {
#ifndef __WIN32__
		char *filename;
		FILE *f;
		void *buf;

		filename = mktemp("/tmp/widelands-sfx.XXXXXXXX");	//manpage recommends a minimum suffix length of 6
		f = fopen(filename, "w+");
		target = SDL_RWFromFP(f, 0);
		buf = malloc(fr->GetSize());

		SDL_RWread(src, buf, fr->GetSize(), 1);
		SDL_RWwrite(target, buf, fr->GetSize(), 1);
		Mix_LoadWAV(filename);

		SDL_RWclose(target);
		SDL_FreeRW(target);
		SDL_FreeRW(src);
		fclose(f);
		free(buf);

		
      unlink( filename );
#else
   }

	SDL_RWclose(src);
	return 0;
}

/** Add exactly one file to the given fxset.
 * \param filename	The effect to be loaded
 * \param fx_name	The fxset to add the file to
 * The file format must be ogg or wav. Otherwise this call will complain, but fail without consequences.
 * \note The complete audio file will be loaded into memory and stays there until program termination
*/
void Sound_Handler::load_one_fx(const string filename, const string fx_name)
{
	FileRead fr;
	Mix_Chunk *m;

	fr.Open(g_fs, filename);
	m = RWopsify_MixLoadWAV(&fr);

	if (m) {
		//make sure that requested FXset exists
		if (fxs.count(fx_name) == 0)
			fxs[fx_name] = new FXset();

		fxs[fx_name]->add_fx(m);
		log(("Loaded sound effect from \"" + filename + "\" for FXset \"" + fx_name + "\"\n").c_str());
	} else {
		char *msg = (char *) malloc(1024 * 1024);
		sprintf(msg, "Sound_Handler: loading sound effect \"%s\" for FXset \"%s\" failed: %s\n",
			filename.c_str(), fx_name.c_str(), strerror(errno));
		log(msg);
		free(msg);
	}
}

/** Calculate  the position of an effect in relation to the visible part of the screen.
 * \param position	Where the event happened (map coordinates)
 * \return position in widelands' game window: left=0, right=254, not in viewport = -1
 * \note This function can also be used to check whether a logical coordinate is visible at all
*/
int Sound_Handler::stereo_position(const Coords position)
{
	int sx, sy;		//screen x,y (without clipping applied, might well be invisible)
	int xres, yres;		//x,y resolutions of game window
	Point vp;
	Interactive_Base *ia;

	assert(the_game);

	ia = the_game->get_iabase();
	assert(ia);
	vp = ia->get_mapview()->get_viewpoint();

	xres = ia->get_xres();
	yres = ia->get_yres();

	the_game->get_map()->get_basepix(position, &sx, &sy);
	sx -= vp.x;
	sy -= vp.y;

	if (sx >= 0 && sx <= xres && sy >= 0 && sy <= yres)	//make sure position is inside viewport
		return sx * 254 / xres;

	return -1;
}

/** Play (one of multiple) sound effect(s) with the given name. The effect(s) must
 * have been loaded before with \ref load_fx.
 * \param fx_name	The identifying name of the sound effect, see \ref load_fx
 * \param map_position  Map coordinates where the event takes place
*/
void Sound_Handler::play_fx(const string fx_name, Coords map_position)
{
	if (map_position==INVALID_POSITION) {
		log("WARNING: play_fx(\"%s\") called without coordinates\n", fx_name.c_str());
		map_position=NO_POSITION;
	}
	
	if (map_position == NO_POSITION)
		play_fx(fx_name, 128);

	play_fx(fx_name, stereo_position(map_position));
}

/** \overload
 * \param fx_name		The identifying name of the sound effect, see \ref load_fx
 * \param stereo_position	position in widelands' game window, see \ref stereo_position
*/
void Sound_Handler::play_fx(const string fx_name, int stereo_position)
{
	Mix_Chunk *m;
	int chan;

	assert(stereo_position>=-1 && stereo_position<=254);

	if (disable_fx)
		return;

	if (fxs.count(fx_name) == 0) {
		log("Sound_Handler: sound effect \"%s\" does not exist!\n", fx_name.c_str());
		return;
	}
	
	log("-------------------------------------------------------------Trying to play %s\n", fx_name.c_str());

	if (fx_name != "create_construction_site") {	//create_construction_site gets played every time
		//TODO: play always should be configurabel in the/a conffile
		//TODO: refine the decision whether to play or not
		if (SDL_GetTicks() < fxs[fx_name]->last_used + 5000)
			return;
	}

	m = fxs[fx_name]->get_fx();
	if (m) {
		if (stereo_position != -1) {
			
			chan = Mix_PlayChannel(-1, m, 0);
			Mix_SetPanning(chan, stereo_position, 254 - stereo_position);
			//TODO: use chan in the fx callback to absoutely ensure that an effect is playing
			//      only once. The "play-or-not" decision is not fit to make *that* choice
			//      Note that a very few effects *always* play, like "start of battle"
		} else {
			//TODO: what to do with fx that happen offscreen?
		}

	} else
		log("Sound_Handler: sound effect \"%s\" exists but contains no files!\n", fx_name.c_str());
}

/** Load a background song. One song can consist of several audio files named
 * FILE_XX.wav (ogg, wav), where XX is between 00 and 99.
 * \param dir		The directory where the audio files reside
 * \param basename	Name from which filenames will be formed (BASENAME_XX.wav);
 * 			also the name used with \ref play_fx
 * \param recursive	\internal
 * This just registers the song, actual loading takes place when \ref Songset::get_song()
 * is called, i.e. when the song is about to be played.\n Supported file formats are wav and ogg.
 * If BASENAME_XX (with any extension) is a directory, effects will be registered recursively.
 * Subdirectories of and files under BASENAME_XX can be named anything you want*/
void Sound_Handler::register_song(const string dir, const string basename, const bool recursive)
{
	filenameset_t files;
	filenameset_t::const_iterator i;

	if (recursive)
		g_fs->FindFiles(dir, "*", &files);
	else
		g_fs->FindFiles(dir, basename + "_??*", &files);

	for (i = files.begin(); i != files.end(); ++i) {
		if (g_fs->IsDirectory(*i)) {
			register_song(*i, basename, true);
		} else {
			if (songs.count(basename) == 0)
				songs[basename] = new Songset();

			songs[basename]->add_song(*i);
			log(("Registered song from file \"" + *i + "\" for songset \"" + basename + "\"\n").c_str());
		}
	}
}

/** Start playing a songset.
 * \param songset_name	The songset to play a song from
 * \param fadein_ms	Song will fade from 0% to 100% during fadein_ms milliseconds
 * 			milliseconds starting from now
 * \note When calling start_music() while music is still fading out from \ref stop_music()
 * or \ref change_music() this function will block until the fadeout is complete*/
void Sound_Handler::start_music(const string songset_name, int fadein_ms)
{
	Mix_Music *m = NULL;

	if (disable_music)
		return;
	if (fadein_ms == 0)
		fadein_ms = 50;	//avoid clicks

	if (Mix_PlayingMusic())
		change_music(songset_name, 0, fadein_ms);

	if (songs.count(songset_name) == 0)
		log("Sound_Handler: songset \"%s\" does not exist!\n", songset_name.c_str());
	else {
		m = songs[songset_name]->get_song();
		if (m) {
			Mix_FadeInMusic(m, 1, fadein_ms);
			current_songset = songset_name;
		} else
			log("Sound_Handler: songset \"%s\" exists but contains no files!\n", songset_name.c_str());
	}
}

/** Stop playing a songset.
 * \param fadeout_ms Song will fade from 100% to 0% during fadeout_ms milliseconds
 * starting from now.*/
void Sound_Handler::stop_music(int fadeout_ms)
{
	if (disable_music)
		return;
	if (fadeout_ms == 0)
		fadeout_ms = 50;	//avoid clicks

	Mix_FadeOutMusic(fadeout_ms);
}

/** Play an other piece of music.
 * This is a member function provided for convenience. It is a wrapper around
 * \ref start_music and \ref stop_music.
 * \param fadeout_ms	Old song will fade from 100% to 0% during fadeout_ms milliseconds
 * 			milliseconds starting from now
 * \param fadein_ms	New song will fade from 0% to 100% during fadein_ms milliseconds
 * 			milliseconds starting from now
 * If songset_name is empty, another song from the currently active songset will be selected*/
void Sound_Handler::change_music(const string songset_name, int fadeout_ms, int fadein_ms)
{
	string s;

	s = songset_name;
	if (s == "")
		s = current_songset;
	else
		current_songset = s;

	if (Mix_PlayingMusic())
		stop_music(fadeout_ms);
	else
		start_music(s, fadein_ms);
}

/** Normal get_* function */
bool Sound_Handler::get_disable_music()
{
	return disable_music;

}

/** Normal get_* function */
bool Sound_Handler::get_disable_fx()
{
	return disable_fx;
}

/** Normal set_* function, but the music must be started/stopped accordingly
 * Also, the new value is written back to the config file right away. It might get lost otherwise.
*/
void Sound_Handler::set_disable_music(bool state)
{
	if (state) {
		stop_music();
		disable_music = true;
	} else {
		disable_music = false;
		start_music(current_songset);
	}

	g_options.pull_section("global")->set_bool("disable_music", state, false);
}

/** Normal set_* function
 * Also, the new value is written back to the config file right away. It might get lost otherwise.
*/
void Sound_Handler::set_disable_fx(bool state)
{
	disable_fx = state;
	g_options.pull_section("global")->set_bool("disable_fx", state, false);
}

/** Callback to notify \ref Sound_Handler that a song has finished playing.
 * Usually, another song from the same songset will be started.\n
 * There is a special case for the intro screen's music: only one song will be
 * played. If the user has not clicked the mouse or pressed escape when the song finishes,
 * Widelands will automatically go on to the main menu.*/
void Sound_Handler::music_finished_callback()
{
	//DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!

	//special case for splashscreen: there, only one song is ever played
	if (g_sound_handler.current_songset == "intro") {
		SDL_Event *e = new SDL_Event();

		e->type = SDL_KEYDOWN;
		e->key.state = SDL_PRESSED;
		e->key.keysym.sym = SDLK_ESCAPE;
		SDL_PushEvent(e);
	} else {		//else just play the next song - see general description for further explanation
		SDL_Event *e = new SDL_Event();

		e->type = SDL_USEREVENT;
		e->user.code = SOUND_HANDLER_CHANGE_MUSIC;
		SDL_PushEvent(e);
	}
}

/** Callback to notify \ref Sound_Handler that a sound effect has finished playing.*/
void Sound_Handler::fx_finished_callback(int channel)
{
	//DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!
}
