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

#include <vector>
#include <map>
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "profile.h"
#include "options.h"
#include "error.h"
#include "random.h"
#include "filesystem.h"

using namespace std;

class Sound_Handler;
extern Sound_Handler* sound_handler;

typedef vector<Mix_Music*> songset_t;

class Sound_Handler {
public:
        //update ctor(), read_config() when adding new songset types
        enum songset_id {NONE, INTRO, MENU, INGAME};
        vector<string> songset_names;

        Sound_Handler();

        void read_config();

        void start_music(const songset_id songset=NONE, int fadein_ms=0);
        void change_music(const songset_id songset=NONE, int fadeout_ms=0, int fadein_ms=0);
        void stop_music(int fadeout_ms=0);

        static void music_finished_callback();

protected:
        Mix_Music* get_mix_music(const songset_id songset);
        void load_one_song(const string filename, const songset_id songset);
        void load_song_dir_recursive(const string filename, const songset_id songset);
        void import_music_class(Section* s, const songset_id songset);

        songset_id current_songset;

        map<songset_id, songset_t::iterator> current_song;
        map<songset_id, songset_t > songs;

        RNG rng;

        bool disable_sound;
        bool random_order;
};

#endif
