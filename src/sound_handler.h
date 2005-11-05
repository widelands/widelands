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

//needed to enable preliminary support for Mix_LoadMUS_RW in SDL_mixer-1.2.6 (see RWopsify_MixLoadWAV())
//TODO: this should really be determined by autoconf or sth. similar
#define USE_RWOPS 1

#include <vector>
#include <map>
#include <SDL.h>
#include <SDL_mixer.h>
#include "random.h"
#include "geometry.h"
#include "game.h"

using namespace std;

class Sound_Handler;
/**\file*/
/** Reference to the global \ref Sound_Handler object*/
extern Sound_Handler* sound_handler;

/** A collection of several pieces of music meant for the same situation.
 * 
 * A Songset encapsulates a number of interchangeable pieces of (background) music, e.g.
 * all songs that might be played while the main menu is being shown. It is possible to 
 * access those songs on after another or in random order. The fact that a Songset really 
 * contains several different songs is hidden from the outside.\n
 * A songset does not contain the audio data itself, to not use huge amounts of memory.
 * Instead, each song is loaded on request and the data is free()d afterwards.*/
class Songset {
public:
	Songset();
	~Songset();
	
	void add_song(string filename);
	Mix_Music* get_song();
	bool empty() {return songs.empty();}
	
protected:
	/** The filenames of all configured songs*/
	vector<string> songs;
	/** Pointer to the song that is currently playing (actually the one that was last started);
	 * needed for linear playback*/
	vector<string>::iterator current_song;
	
	FileRead* fr;
	Mix_Music* m;
	SDL_RWops* rwops;
};

/** A collection of several soundeffects meant for the same event.
 * 
 * An FXset encapsulates a number of interchangeable sound effects, e.g.
 * all effects that might be played when a blacksmith is happily hammering away. It is
 * possible to access the effects on after another or in random order. The fact that an
 * FXset really contains several different effect is hidden from the outside.*/
class FXset {
	friend class Sound_Handler;
public:
	FXset(Uint8 prio=127);
	~FXset();
	
	void add_fx(Mix_Chunk* fx, Uint8 prio=127);
	Mix_Chunk* get_fx();
	bool empty() {return fxs.empty();}

protected:
	/** The collection of sound effects*/
	vector<Mix_Chunk*> fxs;
	
	/** When the effect was played the last time (milliseconds since sdl initialization). Set via SDL_GetTicks()*/
	Uint32 last_used;
	
	/** Minimum time in milliseconds until the effect may be played again */
	Uint32 min_interval;
	
	/** How important is it to play the effect even when others are running already?
	 * Range from 0 (do not play at all if anything else is happening) to 255 (always play,
	 * regardless of other considerations)*/
	Uint8 priority;
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
 * songsets. Each \ref Songset contains references to one or more songs in any format
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
 * must reside directly in the directory 'sounds' and must be named SONGSET_XX.??? where
 * XX is a number from 00 to 99 and ??? is a filename extension. All subdirectories of
 * 'sounds' will be considered to contain ingame music. The subdirectories and the music
 * found in them can be arbitrarily named.
 *
 * \par Sound effects
 * 
 * Buildings and workers can use sound effects in their programs. To do so, use e.g.
 * "playFX blacksmith_hammer"
 * The conf file parser will then load one or more audio files for 'hammering blacksmith'
 * from the building's/worker's configuration directory and store them in an \ref FXset
 * for later access, similar to music stored in songsets. For effects, however, the selection 
 * is always random.
 * 
 * \par Usage of callbacks
 * 
 * SDL_mixer's way to notify the application of important sound events, e.g. that a song is
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
 * Yes, that's just a tad ugly.
 * 
 * No, there's no other way. At least none that I found.
 * 
 * \par Stopping music without blocking
 * 
 * When playing background music with SDL_mixer, we can fade the audio in/out. Unfortunately,
 * Mix_FadeOutMusic will return immediately - but, as the music is not yet stopped, starting
 * a new piece of background music will block. So the choice is to block (directly) after
 * ordering to fade out or indirectly when starting the next piece. Now imagine a fadeout-time
 * of 30 seconds ......
 * 
 * The solution is to work asynchronously which is doable, as we already use a callback to tell
 * us when the audio is \e completely finished. So in \ref stop_music (or \ref change_music )
 * we just start the fadeout. The callback then tells us when the audio has actually stopped
 * and we can start the next music. To differentiate between the to states we can
 * just take a peek with Mix_MusicPlaying if there is music running. To make sure that nothing
 * bad happens, that check is not only required in \ref change_music but also in \ref start_music,
 * which causes the seemingly recursive call to change_music from inside start_music. It really
 * is not recursive, trust me :-)
 * 
 * \todo Bobs want to make noise too, not just workers *g*
 * \todo FX should have a priority (e.g. fights are more important than fishermen)
 * \todo Sound_Handler must not play *all* FX but only a select few
 */
class Sound_Handler {
	friend class Songset;
	friend class FXset;
public:
#define NO_POSITION Coords(-2,-2)
	enum {SOUND_HANDLER_CHANGE_MUSIC=1};
	
        Sound_Handler();
	~Sound_Handler();
	
        void read_config();
	void load_system_sounds();
	
	void load_fx(const string dir, const string basename, const bool recursive=false);
	void play_fx(const string fx_name, const Coords map_position);
	void play_fx(const string fx_name);
	
	void register_song(const string dir, const string basename, const bool recursive=false);
	void start_music(const string songset_name, int fadein_ms=0);
	void stop_music(int fadeout_ms=0);
	void change_music(const string songset_name="", int fadeout_ms=0, int fadein_ms=0);
	
	static void music_finished_callback();
	static void fx_finished_callback(int channel);

	bool get_disable_music() {return disable_music;}
	bool get_disable_fx() {return disable_fx;}
	void set_disable_music(bool state) {disable_music=state;}
	void set_disable_fx(bool state) {disable_fx=state;}
	void toggle_music() {disable_music=!disable_music;}
	void toggle_fx() {disable_fx=!disable_fx;}

	/** The game logic where we can get a mapping from logical to screen coordinates and vice versa*/
	Game* the_game;

protected:
	/** TODO: comment me*/
	Mix_Chunk* RWopsify_MixLoadWAV(FileRead* fr);

	/** TODO: comment me*/
	void load_one_fx(const string filename, const string fx_name);

	/** TODO: comment me*/
	int stereo_position(const Coords position);
	
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
