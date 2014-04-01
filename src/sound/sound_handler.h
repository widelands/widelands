/*
 * Copyright (C) 2005-2008, 2011 by the Widelands Development Team
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

#ifndef SOUND_HANDLER_H
#define SOUND_HANDLER_H

#include <cstring>
#include <map>
#include <string>
#include <vector>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "sound/fxset.h"
#include "io/fileread.h"
#include "logic/widelands_geometry.h"
#include "random.h"

namespace Widelands {class Editor_Game_Base;}
struct Songset;
struct SDL_mutex;

/// How many milliseconds in the past to consider for
/// Sound_Handler::play_or_not()
#define SLIDING_WINDOW_SIZE 20000

extern class Sound_Handler g_sound_handler;

/** The 'sound server' for Widelands.
 *
 * Sound_Handler collects all functions for dealing with music and sound effects
 * in one class. It is similar in task - though not in scope - to well known
 * sound servers like gstreamer, EsounD or aRts. For the moment (and probably
 * forever), the only backend supported is SDL_mixer.
 *
 * \par Music
 *
 * Background music for different situations (e.g. 'Menu', 'Gameplay') is
 * collected in songsets. Each Songset contains references to one or more
 * songs in ogg format. The only ordering inside a soundset is from the order
 * in which the songs were loaded.
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
 * For more information about the naming scheme, see load_fx()
 *
 * You should be using the ogg format for music.
 *
 * \par Sound effects
 *
 * Buildings and workers can use sound effects in their programs. To do so, use
 * e.g. "playFX blacksmith_hammer" in the appropriate conf file. The conf file
 * parser will then load one or more audio files for 'hammering blacksmith'
 * from the building's/worker's configuration directory and store them in an
 * FXset for later access, similar to the way music is stored in songsets.
 * For effects, however, the selection is always random. Sound effects are kept
 * in memory at all times, to avoid delays from disk access.
 *
 * The abovementioned sound effects are synchronized with a work program. It's
 * also possible to have sound effects that are synchronized with a
 * building/worker \e animation. For more information about this look at class
 * AnimationManager.
 *
 * \par Usage of callbacks
 *
 * SDL_mixer's way to notify the application of important sound events, e.g.
 * that a song is finished, are callbacks. While callbacks in and of themselves
 * are a fine thing, they can also be a pain in the body part with which we
 * usually touch our chairs.
 *
 * Problem 1:
 *
 * Callbacks must use global(or static) functions \e but \e not normal member
 * functions of a class. If you must know why: ask google. But how can a
 * static function share data with an instance of it's own class? Usually not at
 * all.
 *
 * Fortunately, g_sound_handler already is a global variable,
 * and therefore accessible to global functions. So problem 1 disappears.
 *
 * Problem 2:
 *
 * Callbacks run in the caller's context. This means that when
 * music_finished_callback() is called, SDL_mixer and SDL_audio <b>will
 * be holding all of their locks!!</b> "So what?", you ask. The above means
 * that one \e must \e not call \b any SDL_mixer functions from inside the
 * callback, otherwise a deadlock is guaranteed. This indirectly does include
 * start_music(), stop_music() and of course change_music().
 * Unfortunately, that's just the functions we'd need to execute from the
 * callback. As if that was not enough, SDL_mixer internally uses
 * two separate threads, so you \e really don't want to play around with
 * locking.
 *
 * The only way around that resctriction is to send an SDL event(SDL_USEREVENT)
 * from the callback (non-sound SDL functions \e can be used). Then, later,
 * the main event loop will process this event \e but \e not in
 * SDL_mixer's context, so locking is no problem.
 *
 * Yes, that's just a tad ugly.
 *
 * No, there's no other way. At least none that I found.
 *
 * \par Stopping music without blocking
 *
 * When playing background music with SDL_mixer, we can fade the audio in/out.
 * Unfortunately, Mix_FadeOutMusic() will return immediately - but, as the music
 * is not yet stopped, starting a new piece of background music will block. So
 * the choice is to block (directly) after ordering to fade out or indirectly
 * when starting the next piece. Now imagine a fadeout-time of 30 seconds ...
 * and the user who is waiting for the next screen ......
 *
 * The solution is to work asynchronously, which is doable, as we already use a
 * callback to tell us when the audio is \e completely finished. So in
 * stop_music() (or change_music()) we just start the fadeout. The
 * callback then tells us when the audio has actually stopped and we can start
 * the next music. To differentiate between the two states we can just take a
 * peek with Mix_MusicPlaying() if there is music running. To make sure that
 * nothing bad happens, that check is not only required in change_music()
 * but also in start_music(), which causes the seemingly recursive call to
 * change_music() from inside start_music(). It really is not recursive, trust
 * me :-)
 *
 * \todo DOC: priorities
 * \todo DOC: play-or-not algorithm
 * \todo Environmental sound effects (e.g. wind)
 * \todo repair and reenable animation sound effects for 1-pic-animations
 * \todo accommodate runtime changes of i18n language
 * \todo ? accommodate sound activation if it was disabled at the beginning
*/
class Sound_Handler
{
	friend struct Songset;
	friend struct FXset;
public:
	Sound_Handler();
	~Sound_Handler();

	void init();
	void shutdown();
	void read_config();
	void load_system_sounds();

	void load_fx_if_needed
		(const std::string & dir,
		 const std::string & filename,
		 const std::string & fx_name);
	void play_fx
		(const std::string & fx_name,
		 Widelands::Coords   map_position,
		 uint8_t             priority = PRIO_ALLOW_MULTIPLE + PRIO_MEDIUM);
	void play_fx
		(const std::string & fx_name,
		 int32_t             stereo_position,
		 uint8_t             priority = PRIO_ALLOW_MULTIPLE + PRIO_MEDIUM);

	void register_song
		(const std::string & dir,
		 const std::string & basename);
	void start_music(const std::string & songset_name, int32_t fadein_ms = 0);
	void stop_music(int32_t fadeout_ms = 0);
	void change_music
		(const std::string & songset_name = std::string(),
		 int32_t             fadeout_ms   = 0,
		 int32_t             fadein_ms    = 0);

	static void music_finished_callback();
	static void fx_finished_callback(int32_t channel);
	void handle_channel_finished(uint32_t channel);

	bool get_disable_music() const;
	bool get_disable_fx   () const;
	int32_t  get_music_volume () const;
	int32_t  get_fx_volume    () const;
	void set_disable_music(bool disable);
	void set_disable_fx   (bool disable);
	void set_music_volume (int32_t volume);
	void set_fx_volume    (int32_t volume);

	/**
	 * Return the max value for volume settings. We use a function to hide
	 * SDL_mixer constants outside of sound_handler.
	 */
	int32_t get_max_volume() const {return MIX_MAX_VOLUME;}

	/** The game logic where we can get a mapping from logical to screen
	 * coordinates and vice vers
	*/
	Widelands::Editor_Game_Base * m_egbase;

	/** Only for buffering the command line option --nosound until real initialization is done.
	 *  And disabling sound on dedicated servers
	 * \see Sound_Handler::Sound_Handler()
	 * \see Sound_Handler::init()
	 * \todo This is ugly. Find a better way to do it
	*/
	bool m_nosound;

	/** Can m_disable_music and m_disable_fx be changed?
	 * true = they mustn't be changed (e.g. because hardware is missing)
	 * false = can be changed at user request
	*/
	bool m_lock_audio_disabling;

protected:
	Mix_Chunk * RWopsify_MixLoadWAV(FileRead &);
	void load_one_fx(const char * filename, const std::string & fx_name);
	int32_t stereo_position(Widelands::Coords position);
	bool play_or_not
		(const std::string & fx_name,
		 int32_t             stereo_position,
		 uint8_t             priority);

	/// Whether to disable background music
	bool m_disable_music;
	/// Whether to disable sound effects
	bool m_disable_fx;
	/// Volume of music (from 0 to get_max_volume())
	int32_t m_music_volume;
	/// Volume of sound effects (from 0 to get_max_volume())
	int32_t m_fx_volume;

	/** Whether to play music in random order
	 * \note Sound effects will \e always be selected at random (inside
	 * their FXset, of course.
	*/
	bool m_random_order;

	/// A collection of songsets
	typedef std::map<std::string, Songset *> Songset_map;
	Songset_map m_songs;

	/// A collection of effect sets
	typedef std::map<std::string, FXset *> FXset_map;
	FXset_map m_fxs;

	/// List of currently playing effects, and the channel each one is on
	/// Access to this variable is protected through m_fx_lock mutex.
	typedef std::map<uint32_t, std::string> Activefx_map;
	Activefx_map m_active_fx;

	/** Which songset we are currently selecting songs from - not regarding
	 * if there actually is a song playing \e right \e now.
	*/
	std::string m_current_songset;

	/** The random number generator.
	 * \note The RNG here \e must \e not be the same as the one for the game
	 * logic!
	*/
	RNG m_rng;

	/// Protects access to m_active_fx between callbacks and main code.
	SDL_mutex * m_fx_lock;
};

#endif
