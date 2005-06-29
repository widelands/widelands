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
#include "profile.h"
#include "options.h"
#include "error.h"
#include "filesystem.h"
#include "map.h"
#include "interactive_base.h"
#include "mapview.h"

/** Delete all songs to avoid memory leaks. This also frees the audio data.*/
Songset::~Songset()
{
	songs.clear();
}

/** Append a song to the end of the songset
 * \param song	The song to append
 * \note The \ref current_song will unconditionally be set to the songset's first song.
 * If you do not want to disturb the (linear) playback order then load all songs before
 * you start playing*/
void Songset::add_song(Mix_Music* song)
{
	songs.push_back(song);
	
	current_song=songs.begin();
}

/** Get a song from the songset. Depending on \ref Sound_Handler::random_order, 
 * the selection will either be random or linear (after last song, will 
 * start again with first.*/
Mix_Music* Songset::get_song()
{
	int songnumber;
	
	if (sound_handler->disable_music || songs.empty())
		return NULL;
	
	if (sound_handler->random_order) {
		songnumber=sound_handler->rng.rand()%songs.size();
		return songs.at(songnumber);
	} else {
		if (current_song==songs.end())
			current_song=songs.begin();	
		return *(current_song++);
	}
}

//--------------------------------------------------------------------------------------

/** Delete all fxs to avoid memory leaks. This also frees the audio data.*/
FXset::~FXset()
{		
	fxs.clear();	
}

/** Append a sound effect to the end of the fxset
 * \param fx	The sound fx to append*/
void FXset::add_fx(Mix_Chunk* fx)
{
	fxs.push_back(fx);
}

/** Get a sound effect from the fxset. \e Which variant of the fx is actually given
 * out is determined at random*/
Mix_Chunk* FXset::get_fx()
{
	int fxnumber;
	
	if (sound_handler->disable_fx || fxs.empty())
		return NULL;
	
	fxnumber=sound_handler->rng.rand()%fxs.size();
	
	return fxs.at(fxnumber);
}

//-----------------------------------------------------------------------------------------

/** Set up \ref Sound_Handler for operation.
 * Initialize RNG, read configuration, set callback for 'song finished playing' and
 * 'fx finished playing'*/
Sound_Handler::Sound_Handler()
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	if ( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) == -1 ) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		throw wexception("Failed to initialize sound system: %s\n", Mix_GetError());
	}
	
	current_songset="";
	read_config();
	rng.seed(SDL_GetTicks()); //that's still somewhat predictable, but it's just to avoid identical
				  //playback patterns
	
	Mix_HookMusicFinished(Sound_Handler::music_finished_callback);
	Mix_ChannelFinished(Sound_Handler::fx_finished_callback);
}

/** Housekeeping: unset hooks. Audio data will be freed automagically by the \ref Songset and \ref FXset destructors*/
Sound_Handler::~Sound_Handler()
{
	Mix_ChannelFinished(NULL);
	Mix_HookMusicFinished(NULL);
	
	Mix_CloseAudio();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

/** Read the main config file, load background music and systemwide sound effects*/
void Sound_Handler::read_config()
{
	Section* s;
	
	s=g_options.pull_section("global");
	
	disable_music=s->get_bool("disable_music",false);
	disable_fx=s->get_bool("disable_fx",false);
	random_order=s->get_bool("random_order", true);
	
	load_song("music", "intro");
	load_song("music", "menu");
	load_song("music", "ingame");
	//TODO: load complete directories as ingame songs

	//only systemwide fx, worker/building fx will be loaded by their respective conf-file parsers
	load_fx("sound","create_construction_site");
}

/** Load a sound effect. One sound effect can consist of several audio files named
 * EFFECT_XX.wav (ogg, wav), where XX is between 00 and 99.
 * \param dir		The directory where the audio files reside
 * \param basename	Name from which filenames will be formed (BASENAME_XX.wav);
 * 			also the name used with \ref play_fx */
void Sound_Handler::load_fx(const string dir, const string basename)
{	
	filenameset_t files;
	filenameset_t::const_iterator i;
	
	g_fs->FindFiles(dir, basename+"_??.*", &files);
	
	for (i=files.begin(); i!=files.end(); ++i)
		load_one_fx(*i, basename);
}

/** Add exactly one file to the given fxset.
 * \param filename	The effect to be loaded
 * \param fx_name	The fxset to add the file to
 * The file format must be ogg or wav. Otherwise this call will complain, but fail without consequences.
 * \note The complete audio file will be loaded into memory and stay there until program termination*/
void Sound_Handler::load_one_fx(const string filename, const string fx_name)
{
	FileRead fr;
	Mix_Chunk* m;
	
	fr.Open(g_fs, filename);
	
	m=Mix_LoadWAV_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1); //SDL_mixer will free the RWops itself
	
	if (m) {		
		//make sure that requested FXset exists
		if (fxs.count(fx_name)==0)
			fxs[fx_name]=new FXset();
		
		fxs[fx_name]->add_fx(m);
		log(("Loaded sound effect \""+fx_name+"\" from \""+filename+"\"\n").c_str());
	} else
		log(("Sound_Handler: loading sound effect \""+fx_name+"\" from \""+filename+"\" failed!\n").c_str());
}

/** Find out whether a logical coordinate is currently visible on screen*/
bool Sound_Handler::coords_visible(const Coords position)
{
	int sx, sy; //screen x,y (without clipping applied, might well be invisible)
	Interactive_Base* ia;
	Point vp;
	
	if (the_game) {
		ia=the_game->get_iabase();
		vp=ia->get_mapview()->get_viewpoint();
		
		the_game->get_map()->get_basepix(position, &sx, &sy);
		sx -= vp.x;
		sy -= vp.y;
		
		if ((sx>0 && sx<ia->get_xres()) && (sy>0 && sy<ia->get_yres()))
			return true;
	}
		
	return false;
}


/** Play (one of multiple) sound effect(s) with the given name. The effect(s) must
 * have been loaded before with \ref load_fx.
 * \param fx_name	The identifying name of the sound effect, see \ref load_fx
 * \param map_position  Logical (not screen) coordinates where the event takes place*/
void Sound_Handler::play_fx(const string fx_name, const Coords map_position)
{
	Mix_Chunk* m;
		
	if (disable_fx) return;
	
	if (map_position==NO_POSITION)
		//make sure that it's centered and full volumes
		;
	
	if (fxs.count(fx_name)==0)
		log("Sound_Handler: sound effect \"%s\" does not exist!\n", fx_name.c_str());
	else {		
		m=fxs[fx_name]->get_fx();	
		if(m) {
			if (coords_visible(map_position))
				Mix_PlayChannel(-1, m, 0);
		} else
			log("Sound_Handler: sound effect \"%s\" exists but contains no files!\n", fx_name.c_str());
	}
}

/** \overload*/
void Sound_Handler::play_fx(const string fx_name)
{
	log("WARNING: play_fx(\"%s\") called without coordinates\n", fx_name.c_str());
	play_fx(fx_name, NO_POSITION);
}

/** Load a background song. One song can consist of several audio files named
 * FILE_XX.wav (ogg, wav), where XX is between 00 and 99.
 * \param dir		The directory where the audio files reside
 * \param basename	Name from which filenames will be formed (BASENAME_XX.wav);
 * 			also the name used with \ref play_fx */
void Sound_Handler::load_song(const string dir, const string basename)
{	
	filenameset_t files;
	filenameset_t::const_iterator i;
	
	g_fs->FindFiles(dir, basename+"_??.*", &files);
	
	for (i=files.begin(); i!=files.end(); ++i)
		load_one_song(*i, basename);
}

/** Add exactly one file to the given songset.
 * \param filename	The song to be loaded
 * \param songset_name	The songset to add the file to
 * \note The file formats are a subset of those supported by SDL_mixer, namely: wav and ogg. If you feed it
 * other files, this call will complain, but fail without consequences.
 * \note Unlike sound effects, songs will not be buffered in memory but are read from disk when playing*/
void Sound_Handler::load_one_song(const string filename, const string songset_name)
{
	FileRead* fr=NULL;
	Mix_Music* m=NULL;
	SDL_RWops* rwops=NULL;			
	
	try {
		fr=new FileRead();
		fr->Open(g_fs, filename); //TODO: close again, part of keeping only one file open at a time
		
		rwops=SDL_RWFromMem(fr->Data(0), fr->GetSize());

#ifdef __WIN32__
#warning Mix_LoadMUS_RW is not available under windows!!!
		m=NULL;
#else
		m=Mix_LoadMUS_RW(rwops); //TODO: SDL_mixer does not(??) free this RWop itself???
#endif	

#ifdef OLD_SDL_MIXER
#warning Please update your SDL_mixer library to at least version 1.2.6!!!
		m=Mix_LoadMUS(filename.c_str());
#else
		m=Mix_LoadMUS_RW(rwops); //TODO: SDL_mixer does not(??) free this RWop itself???
#endif
			
		if (m) {
			//make sure the required Songset exists
			if (songs.count(songset_name)==0)
				songs[songset_name]=new Songset();
			
			songs[songset_name]->add_song(m);
			log(("Loaded song \""+songset_name+"\" from \""+filename+"\"\n").c_str());
		} else {
			log(("Sound_Handler: loading song \""+songset_name+"\" from \""+filename+"\" failed!\n").c_str());
			log("%s\n",Mix_GetError());
		}
	}
	catch (...) {
		if (m)
			free(m);
		if (rwops) SDL_FreeRW(rwops);
		if (fr) delete fr;
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
        Mix_Music* m=NULL;
	
	if (disable_music)
		return;	
	if (fadein_ms==0) fadein_ms=50;  //avoid clicks
	
	if (Mix_PlayingMusic())
		change_music(songset_name, 0, fadein_ms);
	
	if (songs.count(songset_name)==0)
		log("Sound_Handler: songset \"%s\" does not exist!\n", songset_name.c_str());
	else {
		m=songs[songset_name]->get_song();	
		if (m) {
			Mix_FadeInMusic(m, 1, fadein_ms);
			current_songset=songset_name;
		} else
			log("Sound_Handler: songset \"%s\" exists but contains no files!\n", songset_name.c_str());
	}
}

/** Stop playing a songset.
 * \param fadeout_ms Song will fade from 100% to 0% during fadeout_ms milliseconds
 * starting from now.*/
void Sound_Handler::stop_music(int fadeout_ms)
{
        if (disable_music) return;
        if (fadeout_ms==0) fadeout_ms=50;  //avoid clicks
	
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
	
	s=songset_name;	
	if (s=="")
		s=current_songset;
	else
		current_songset=s;
	
	if (Mix_PlayingMusic())
		stop_music(fadeout_ms);
	else
		start_music(s, fadein_ms);
}

/** Callback to notify \ref Sound_Handler that a song has finished playing.
 * Usually, another song from the same songset will be started.\n
 * There is a special case for the intro screen's music: here, only one song will be
 * played. If the user has not clicked the mouse or pressed escape when the song finishes,
 * Widelands will automatically go on to the main menu.*/
void Sound_Handler::music_finished_callback()
{
	//DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!
	
        assert(sound_handler); //yes, I'm paranoid
	
        //special case for splashscreen: there, only one song is ever played
        if (sound_handler->current_songset=="intro") {
	        SDL_Event* e=new SDL_Event();

                e->type=SDL_KEYDOWN;
                e->key.state=SDL_PRESSED;
                e->key.keysym.sym=SDLK_ESCAPE;
                SDL_PushEvent(e);
	} else { //else just play the next song - see general description for further explanation
		SDL_Event* e=new SDL_Event();
		
		e->type=SDL_USEREVENT;	
		e->user.code=SOUND_HANDLER_CHANGE_MUSIC;
		SDL_PushEvent(e);
	}
}

/** Callback to notify \ref Sound_Handler that a sound effect has finished playing.*/
void Sound_Handler::fx_finished_callback(int channel)
{
	//DO NOT CALL SDL_mixer FUNCTIONS OR SDL_LockAudio FROM HERE !!!
	
	assert(sound_handler); //yes, I'm paranoid
	
	//TODO: implement me
}
