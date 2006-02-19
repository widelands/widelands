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

//config.h *must* be included before SDL_mixer.h due to USE_RWOPS !!
#include "config.h"
#include <vector>
#include <map>
#include <SDL.h>
#include <SDL_mixer.h>
#include "random.h"
#include "geometry.h"
#include "game.h"

using namespace std;

/// Predefined priorities for easy reading
/// \warning DO NOT CHANGE !! The values have meaning beyond just being numbers
//@{
#define PRIO_ALWAYS_PLAY 255
#define PRIO_ALLOW_MULTIPLE 128
#define PRIO_MEDIUM 63
//@}

/// How many milliseconds in the past to consider for \ref Sound_Handler::play_or_not()
#define SLIDING_WINDOW_SIZE 300000

/// Non-existent 'positions'(=logical map coordinates) with special meaning
//@{
/// Explicit "no position given on purpose"
#define NO_POSITION Coords(-1,-1)
/// No position given (by accident?); like a NULL pointer
#define INVALID_POSITION Coords(-2,-2)
//@}

class Sound_Handler;
///\file

/** The global \ref Sound_Handler object
 * 
 * The sound handler is a static object because otherwise it'd be quite
 * difficult to pass the --nosound command line optio
*/
extern Sound_Handler g_sound_handler;

/** A collection of several pieces of music meant for the same situation.
 *
 * A Songset encapsulates a number of interchangeable pieces of (background) 
 * music, e.g. all songs that might be played while the main menu is being
 * shown. It is possible to access those songs one after another or in
 * random order. The fact that a Songset really contains several different
 * songs is hidden from the outside.\n
 * A songset does not contain the audio data itself, to not use huge amounts of
 * memory. Instead, each song is loaded on request and the data is free()d
 * afterwards
*/
class Songset
{
public:
	Songset();
	~Songset();

	void add_song(const string filename);
	Mix_Music *get_song();
	bool empty() {return m_songs.empty();}

protected:
	/// The filenames of all configured songs
	vector < string > m_songs;

	/** Pointer to the song that is currently playing (actually the one that
	 * was last started); needed for linear playback
	*/
	vector < string >::iterator m_current_song;

	/// The current song
	Mix_Music *m_m;

	/** File reader object to fetch songs from disc when they start playing.
	 * Do not create this for each load, it's a major hassle to code.
	 * \sa m_rwops	
	 * \sa get_song()
	*/
	FileRead *m_fr;

	/** RWops object to fetch songs from disc when they start playing.
	 * Do not create this for each load, it's a major hassle to code.
	 * \sa m_fr 
	 * \sa get_song()
	*/
	SDL_RWops *m_rwops;
};

/** A collection of several sound effects meant for the same event.
 *
 * An FXset encapsulates a number of interchangeable sound effects, e.g.
 * all effects that might be played when a blacksmith is happily hammering away.
 * It is possible to select the effects one after another or in random order.
 * The fact that an FXset really contains several different effects is hidden
 * from the outside
*/
class FXset
{
	friend class Sound_Handler;
public:
	FXset(Uint8 prio = PRIO_MEDIUM);
	~FXset();

	void add_fx(Mix_Chunk * fx, Uint8 prio = PRIO_MEDIUM);
	Mix_Chunk *get_fx();
	bool empty() {return m_fxs.empty();}

protected:
	/// The collection of sound effects
	vector < Mix_Chunk * >m_fxs;

	/** When the effect was played the last time (milliseconds since SDL
	 * initialization). Set via SDL_GetTicks()
	*/
	Uint32 m_last_used;

	/** How important is it to play the effect even when others are running
	 * already?
	 * 
	 * Value 0-127: probability between 0.0 and 1.0, only one instance can
	 * be playing at any time
	 * 
	 * Value 128-254: probability between 0.0 and 1.0, many instances can
	 * be playing at any time
	 * 
	 * Value 255: always play; unconditional
	*/
	Uint8 m_priority;
};

/** The 'sound server' for Widelands.
 *
 * Sound_Handler collects all functions for dealing with music and sound effects
 * in one class. It is similar in task - though not in scope - to well known
 * sound servers like gstreamer, EsounD or aRts. For the moment (and probably 
 * forever), the only backend supported is SDL_mixer.
 *
 * The Sound_Handler is (obviously) a singleton object and is therefore stored
 * in a global variable \ref g_sound_handler to be easily accessible by other
 * classes.
 *
 * \par Music
 *
 * Background music for different situations (e.g. 'Menu', 'Gameplay') is 
 * collected in songsets. Each \ref Songset contains references to one or more
 * songs in any format understood by SDL_mixer (e.g mp3, ogg, wav). The only
 * ordering inside a soundset is from the order in which the songs were
 * loaded.
 *
 * Other classes can request to start or stop playing a certain songset,
 * changing the songset is provided as a convenience method. It is also
 * possible to switch to some other piece inside the same songset - but there
 * is \e no control over \e which song out of a songset gets played. The
 * selection is either linear (the order in which the songs were loaded) or
 * completely random.
 *
 * The files for the predefined system songsets
 * \li \c intro
 * \li \c menu
 * \li \c ingame
 *
 * must reside directly in the directory 'sounds' and must be named 
 * SONGSET_XX.??? where XX is a number from 00 to 99 and ??? is a filename
 * extension. All subdirectories of 'sounds' will be considered to contain
 * ingame music. The the music and sub-subdirectories found in them can be
 * arbitrarily named. This means: everything below sound/ingame_01 can have
 * any name you want. All audio files below sound/ingame_01 will be played as
 * ingame music.
 * 
 * \note You should only be using the ogg format for music.
 *
 * \par Sound effects
 *
 * Buildings and workers can use sound effects in their programs. To do so, use 
 * e.g. "playFX blacksmith_hammer" in the appropriate conf file. The conf file
 * parser will then load one or more audio files for 'hammering blacksmith'
 * from the building's/worker's configuration directory and store them in an 
 * \ref FXset for later access, similar to the way music is stored in songsets.
 * For effects, however, the selection is always random. Sound effects are kept
 * in memory at all times, to avoid delays from disk access.
 *
 * The abovementioned sound effects are synchronized with a work program. It's
 * also possible to have sound effects that are synchronized with a 
 * building/worker \e animation. For more information about this look at class
 * \ref AnimationManager.
 * 
 * \note You should only be using the ogg format for sound effects.
 *
 * \par Usage of callbacks
 *
 * SDL_mixer's way to notify the application of important sound events, e.g. 
 * that a song is finished, are callbacks. While callbacks in and of themselves
 * are a fine thing, they can also be a pain in the body part with which we
 * usually touch our chairs.
 *
 * Problem 1:\n
 * Callbacks must use global(or static) functions \e but \e not normal member 
 * functions of a class. If you must know why: ask google. But how can a
 * static function share data with an instance of it's own class? Usually not at
 * all.\n
 * Fortunately, \ref g_sound_handler already is a global variable,
 * and therefore accessible to global functions. So problem 1 disappears.
 *
 * Problem 2:\n
 * Callbacks run in the caller's context. This means that when 
 * \ref music_finished_callback() is called, SDL_mixer and SDL_audio <b>will
 * be holding all of their locks!!</b> "So what?", you ask. The above means
 * that one \e must \e not call \b any SDL_mixer functions from inside the
 * callback, otherwise a deadlock is guaranteed. This indirectly does include
 * \ref start_music, \ref stop_music and of course \ref change_music.
 * Unfortunately, that's just the functions we'd need to execute from the
 * callback. As if that was not enough, SDL_mixer internally uses
 * two seperate threads, so you \e really don't want to play around with
 * locking.
 *
 * The only way around that resctriction is to send an SDL event(SDL_USEREVENT) 
 * from the callback (non-sound SDL functions \e can be used). Then, later,
 * the main event loop (system.cc) will process this event \e but \e not in 
 * SDL_mixer's context, so locking is no problem.
 *
 * Yes, that's just a tad ugly.
 *
 * No, there's no other way. At least none that I found.
 *
 * \par Stopping music without blocking
 *
 * When playing background music with SDL_mixer, we can fade the audio in/out. 
 * Unfortunately, Mix_FadeOutMusic will return immediately - but, as the music
 * is not yet stopped, starting a new piece of background music will block. So
 * the choice is to block (directly) after ordering to fade out or indirectly
 * when starting the next piece. Now imagine a fadeout-time of 30 seconds ...
 * and the user who is waiting for the next screen ......
 *
 * The solution is to work asynchronously, which is doable, as we already use a 
 * callback to tell us when the audio is \e completely finished. So in 
 * \ref stop_music (or \ref change_music ) we just start the fadeout. The
 * callback then tells us when the audio has actually stopped and we can start
 * the next music. To differentiate between the two states we can just take a
 * peek with Mix_MusicPlaying() if there is music running. To make sure that
 * nothing bad happens, that check is not only required in \ref change_music
 * but also in \ref start_music, which causes the seemingly recursive call to
 * change_music from inside start_music. It really is not recursive, trust 
 * me :-)
 * 
 * \todo Describe priorities
 * \todo Describe play-or-not algo
 * \todo Internationalized sound effects/music
 * \todo Environmental sound effects (e.g. wind)
 * \todo repair and reenable animation sound effects for 1-pic-animations
*/
class Sound_Handler
{
	friend class Songset;
	friend class FXset;
public:
	/// Constants for event loop interaction
	//// \sa "Usage of callbacks" TODO: how do I get this link?
	enum { SOUND_HANDLER_CHANGE_MUSIC = 1 };

	Sound_Handler();
	~Sound_Handler();

	void init();
	void read_config();
	void load_system_sounds();

	void load_fx(const string dir, const string basename,
	             const bool recursive = false);
	void play_fx(const string fx_name, Coords map_position=INVALID_POSITION,
	             const uint priority=PRIO_ALLOW_MULTIPLE+PRIO_MEDIUM);
	void play_fx(const string fx_name, const int stereo_position,
	             const uint priority=PRIO_ALLOW_MULTIPLE+PRIO_MEDIUM);

	void register_song(const string dir, const string basename,
	                   const bool recursive = false);
	void start_music(const string songset_name, int fadein_ms = 0);
	void stop_music(int fadeout_ms = 0);
	void change_music(const string songset_name = "", int fadeout_ms = 0,
	                  int fadein_ms = 0);

	static void music_finished_callback();
	static void fx_finished_callback(int channel);
	void handle_channel_finished(uint channel);

	bool get_disable_music();
	bool get_disable_fx();
	void set_disable_music(bool disable);
	void set_disable_fx(bool disable);

	/** The game logic where we can get a mapping from logical to screen
	 * coordinates and vice vers
	*/
	Game *m_the_game;

	/** Only for buffering the command line option --nosound until real
	 * intialization is done
	 * \see Sound_Handler::Sound_Handler()
	 * \see Sound_Handler::init()
	 * \todo This is ugly. Find a better way to do it
	*/
	bool m_nosound;

	/** Can \ref m_disable_music and \ref m_disable_fx be changed?
	 * true = they mustn't be changed (e.g. because hardware is missing)
	 * false = can be changed at user request
	*/
	bool m_lock_audio_disabling;

protected:
	Mix_Chunk * RWopsify_MixLoadWAV(FileRead * fr);
	void load_one_fx(const string filename, const string fx_name);
	int stereo_position(const Coords position);
	bool play_or_not(const string fx_name,const int stereo_position,
	                 const uint priority);

	/// Whether to disable background music
	bool m_disable_music;
	/// Whether to disable sound effects
	bool m_disable_fx;

	/** Whether to play music in random order
	 * \note Sound effects will \e always be selected at random (inside 
	 * their \ref FXset, of course
	*/
	bool m_random_order;

	/// A collection of songsets
	map < string, Songset * >m_songs;

	/// A collection of effect sets
	map < string, FXset * >m_fxs;

	/// List of currently playing effects, and the channel each one is on
	map<uint, string>m_active_fx;

	/** Which songset we are currently selecting songs from - not regarding
	 * if there actually is a song playing \e right \e now
	*/
	string m_current_songset;

	/** The random number generator.
	 * \note The RNG here \e must \e not be the same as the one for the game 
	 * logic!
	*/
	RNG m_rng;
};

#endif
