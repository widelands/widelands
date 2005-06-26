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

/** Append a song to the end of the songset
 * \param song	The song to append
 * \note The \ref current_song will unconditionally be set to the songset's first song.
 * If you do not want to disturb the (linear) playback order then load all songs before
 * you start playing*/
void Songset::add_song(Mix_Music* song)
{
	songs.push_back(song);
	
	current_song=songs.begin();
	
	//TODO: remove songs in destructor to not leak memory
}

/** Get a song from the songset. Depending on \ref random_order, 
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

/** Append a sound effect to the end of the fxset
 * \param fx	The sound fx to append*/
void FXset::add_fx(Mix_Chunk* fx)
{
	fxs.push_back(fx);
	//TODO: remove songs in destructor to not leak memory
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
	//TODO:?? Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 32, 512);
	
	current_songset="";
	read_config();
	rng.seed(SDL_GetTicks()); //that's still somewhat predictable, but it's just to avoid identical
				  //playback patterns
	
	Mix_HookMusicFinished(Sound_Handler::music_finished_callback);
	Mix_ChannelFinished(Sound_Handler::fx_finished_callback);
}

/** Housekeeping: unset hooks, free all sound data*/
Sound_Handler::~Sound_Handler()
{
	Mix_ChannelFinished(NULL); //unregister callback
	Mix_HookMusicFinished(NULL); //unregister callback
	
	//TODO: actually *do* free :)
}

/** Read the main config file, load background music and systemwide sound effects*/
void Sound_Handler::read_config()
{
	Section* s;
	
	s=g_options.pull_section("global");
	
	disable_music=s->get_bool("disable_music",false);
	disable_fx=s->get_bool("disable_soundfx",false);
	random_order=s->get_bool("random_order", true);
	
	load_song("music", "intro");
	load_song("music", "menu");
	load_song("music", "ingame");
	//TODO: load complete directories as ingame songs

	//only systemwide fx, worker/building fx will be loaded by their respective conf-file parsers
	load_fx("sounds","create_construction_site");
}

/** Load a sound effect. One sound effect can consist of several audio files named
 * EFFECT_XX.wav (ogg, mp3, ...), where XX is between 00 and 99.
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
 * \param songset	The fxset to add the file to
 * \note The file format must be supported by SDL_mixer, this is currently the case for
 * wave, aiff, riff, ogg and voc. Otherwise this call will complain, but fail without consequences.
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

/** Play (one of multiple) sound effect(s) with the given name. The effect(s) must
 * have been loaded before with \ref load_fx.
 * \param fx_name	The identifying name of the sound effect, see \ref load_fx*/
void Sound_Handler::play_fx(const string fx_name)
{
	Mix_Chunk* m;
	
	if (disable_fx) return;
	
	if (fxs.count(fx_name)==0)
		log("Sound_Handler: sound effect \"%s\" does not exist!\n", fx_name.c_str());
	else {		
		m=fxs[fx_name]->get_fx();	
		if(m)
			Mix_PlayChannel(-1, m, 0);
		else
			log("Sound_Handler: sound effect \"%s\" exists but contains no files!\n", fx_name.c_str());
	}
}

/** Load a background song. One song can consist of several audio files named
 * FILE_XX.wav (ogg, wav, perhaps midi(?)), where XX is between 00 and 99.
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
 * \param songset	The songset to add the file to
 * \note The file formats are a subset of those supported by SDL_mixer, namely: wave and ogg. If you feed it
 * other files, this call will complain, but fail without consequences.
 * \note Unlike sound effects, songs will not be buffered in memory but are read from disk when playing*/
void Sound_Handler::load_one_song(const string filename, const string songset_name)
{
	FileRead* fr;
	Mix_Music* m;
	SDL_RWops* rwops;			
	
	fr=new FileRead();
	fr->Open(g_fs, filename);
	
	rwops=SDL_RWFromMem(fr->Data(0), fr->GetSize());

	m=Mix_LoadMUS_RW(rwops); //TODO: SDL_mixer does not(??) free this RWop itself???
		
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

/** Start playing a songset.
 * \param songset	The songset to play a song from
 * \param fadein_ms	Song will fade from 0% to 100% during fadein_ms
 * 			milliseconds starting from now
 * \attention Will block until fadein is complete!
 * \todo Make nonblocking*/
void Sound_Handler::start_music(const string songset_name, int fadein_ms)
{
        Mix_Music* m=NULL;
	
	if (disable_music)
		return;	
	if (fadein_ms==0)
		fadein_ms=50;  //avoid clicks
	
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
 * \param fadeout_ms Song will fade from 100% to 0% during faedout_ms
 * starting from now.
 * \attention Will block until fadeout is complete!
 * \todo Make nonblocking*/
void Sound_Handler::stop_music(int fadeout_ms)
{
        if (disable_music) return;
        if (fadeout_ms==0) fadeout_ms=50;  //avoid clicks
	
	Mix_FadeOutMusic(fadeout_ms);
	
	//TODO: make nonblocking
        while( Mix_FadingMusic()==MIX_FADING_OUT ); //wait until music has stopped
}

/** Play music from a different songset.
 * This is a member function provided for convenience. It is a wrapper around
 * \ref start_music and \ref stop_music.
 * If songset_name is empty, another song from the currently active songset will be selected*/
void Sound_Handler::change_music(const string songset_name, int fadeout_ms, int fadein_ms)
{
	string s;
	
	s=songset_name;	
	if (s=="") s=current_songset;
	
	stop_music(fadeout_ms);
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
		e->user.code=CHANGE_MUSIC;
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
