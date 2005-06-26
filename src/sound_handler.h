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

#ifndef SOUND_HANDLER
#define SOUND_HANDLER

//next one needed to enable preliminary support for Mix_LoadMUS_RW in SDL_mixer
#define USE_RWOPS

#include <vector>
#include <map>
#include <assert.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_thread.h>
#include "profile.h"
#include "options.h"
#include "error.h"
#include "random.h"
#include "filesystem.h"

using namespace std;

class Sound_Handler;

/**\file*/
/** Reference to the global \ref Sound_Handler object*/
extern Sound_Handler* sound_handler;

/** A collection of several pieces of music meant for the sam situation.
 * 
 * A Songset encapsulates a number of interchangeable pieces of (background) music, e.g.
 * all songs that might be played while the main menu is being shown. It is possible to 
 * access those songs on after another or in random order. The fact that a Songset really 
 * contains several different songs is hidden from the outside.*/
class Songset {
public:
	void add_song(Mix_Music* song);
	Mix_Music* get_song();
	bool empty() {return songs.empty();}
	
protected:
	/** The collection of songs*/
	vector<Mix_Music*> songs;
	/** Pointer to the song that is currently playing (actually the one that was last started);
	 * needed for linear playback*/
	vector<Mix_Music*>::iterator current_song;
};

/** A collection of several soundeffects meant for the same event.
 * 
 * An FXset encapsulates a number of interchangeable sound effects, e.g.
 * all effects that might be played when a blacksmith is happily hammering away. It is
 * possible to access the effects on after another or in random order. The fact that an
 * FXset really contains several different effect is hidden from the outside.*/
class FXset {
public:
	void add_fx(Mix_Chunk* fx);
	Mix_Chunk* get_fx();
	bool empty() {return fxs.empty();}

protected:
	/** The collection of sound effects*/
	vector<Mix_Chunk*> fxs;
};

/** The 'sound server' for Widelands.
 * 
 * Sound_Handler collects all functions for dealing with music and sound effects
 * in one class. It is similar in task - though not in scope - to well known
 * sound servers like gstreamer, EsounD or aRts. For the moment (and probably forever),
 * the only backend supported is SDL_mixer.
 * 
 * The Sound_Handler is (obviously) a singleton object and is therefore stored
 * in a global variable to be easily accessible by other classes.
 * 
 * \par Music
 * 
 * Background music for different situations (e.g. 'Menu', 'Gameplay') is collected in
 * songsets. Each \ref songset contains references to one or more songs in any format
 * understood by SDL_mixer (e.g mp3, ogg, wav). The only ordering inside a soundset is
 * from the order in which the songs were loaded.
 * 
 * Other classes can request to start or stop playing a certain songset, changing the
 * songset is provided as a convenience method. It is also possible to switch to some
 * other piece inside the same songset - but here is \e no control over \e which song
 * out of a songset gets played. The selection is either linear (the order in which the songs
 * were loaded) or completely random.
 * 
 * The files for the predefined system songsets
 * \li \c intro
 * \li \c menu
 * \li \c ingame
 * 
 * must reside directly in the directory 'music' and must be named SONGSET_XX.??? where
 * XX is a number from 00 to 99 and ??? is a filename extension. All subdirectories of
 * 'sounds' will be considered to contain ingame music. The subdirectories and the music
 * found in them can be arbitrarily named.
 *
 * \par Sound effects
 * 
 * Buildings and workers can use sound effects in their programs. To do so, use the command
 * \example playFX blacksmith_hammer
 * The conf file parser will then load one or more audio files for 'hammering blacksmith'
 * from the building's/worker's configuration directory and store them in an \ref FXset
 * for later access, similar to music stored in songsets. For effects, however, the selection 
 * is always random.
 * 
 * \par Usage of callbacks
 * 
 * SDL_mixers way to notify the application of important sound events, e.g. that a song is
 * finished, are callbacks. While callbacks in and of themselves are a fine thing, they can
 * also be a pain in the body part with which we usually touch our chairs.
 * 
 * Problem 1:\n
 * Callbacks must use global(or static) functions \e but \e not normal member functions of a
 * class. If you must know why: ask google. But how can a static function share data with it's
 * own class? Usually not at all. Fortunately, \ref sound_handler already is a global variable,
 * and therefore accessible to global functions. So problem 1 disappears.
 * 
 * Problem 2:\n
 * Callbacks run in the caller's context. This means that when \ref music_finished_callback()
 * is called, SDL_mixer and SDL_audio <b>will be holding all of their locks!!</b> "So what?",
 * you ask. The above means that one \e must \e not call \b any SDL_mixer functions from inside the 
 * callback, otherwise a deadlock is guaranteed. This indirectly does include \ref start_music,
 * \ref stop_music and of course \ref change_music. Unfortunately, that's just the functions
 * we'd need to execute from the callback. As if that was not enough, SDL_mixer internally uses
 * two seperate threads, so you \e really don't want to play around with locking.
 * 
 * The only way around that resctriction is to send an SDL event (SDL_USEREVENT) from the
 * callback (non-sound SDL functions \e can be used) and handle the event inside the main loop
 * (system.cc).
 * 
 * Yeah, that's just a better expression for "polling". And for "ugly".
 * 
 * No, there's no other way. At least none that I found.
 * 
 * \todo Fading music in and out should not block
 * \todo Bobs want to make noise too, not just workers *g*
 * \todo Sound_Handler must actually select the fx
 * \todo Stereo for FX
 * \todo FX should have a priority (e.g. fights are more important than fishermen)
 * \todo Sound_Handler must not play *all* FX but only a select few
 */
class Sound_Handler {
	friend class Songset;
	friend class FXset;
public:
	enum {CHANGE_MUSIC=1};
	
        Sound_Handler();
	~Sound_Handler();
	
        void read_config();
	
	void load_fx(const string dir, const string basename);
	void play_fx(const string fx_name);
	
	void load_song(const string dir, const string basename);
	void start_music(const string songset_name, int fadein_ms=0);
	void stop_music(int fadeout_ms=0);
	void change_music(const string songset_name="", int fadeout_ms=0, int fadein_ms=0);
	
	static void music_finished_callback();
	static void fx_finished_callback(int channel);

protected:
	void load_one_fx(const string filename, const string fx_name);
	void load_one_song(const string filename, const string songset_name);
	
	/** Whether to disable background music*/
	bool disable_music;
	
	/** Whether to disable sound effects*/
	bool disable_fx;
	
	/** Whether to play music in random order
	 * \note Sound effects will \e always be selected at random (inside their \ref FXset, of course)*/
	bool random_order;
	
	/** A collection of songsets*/
	map<string, Songset*> songs;
	
	/** A collection of effect sets*/
	map<string, FXset*> fxs;
	
	/** Which songset we are currently selecting songs from - not regarding if there actually is a song
	 * playing \e right \e now*/
	string current_songset;

	/** The random number generator*/
        RNG rng;
};

#endif
