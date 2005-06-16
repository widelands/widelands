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

Sound_Handler::Sound_Handler()
{
        songset_names.push_back("none");
        songset_names.push_back("intro");
        songset_names.push_back("menu");
        songset_names.push_back("ingame");

        read_config();
        current_songset=NONE;
        rng.seed(12345); //TODO: use a seed that is a little bit more random ;-)

        if (!disable_sound)
                Mix_HookMusicFinished(Sound_Handler::music_finished_callback);
}

void Sound_Handler::load_one_song(const string filename, const songset_id songset)
{
        Mix_Music* m=Mix_LoadMUS(filename.c_str());

        if (m) {
                songs[songset].push_back(m);
                log(("Loaded "+songset_names.at(songset)+" song \""+filename+"\"\n").c_str());
        } else
                log(("Sound_Handler: Could not load "+songset_names.at(songset)+" song \""+filename+"\"\n").c_str());
}

void Sound_Handler::load_song_dir_recursive(const string filename, const songset_id songset)
{
        filenameset_t files;
        filenameset_t::const_iterator i;

        if (g_fs->IsDirectory(filename)) {
                g_fs->FindFiles(filename, "*", &files);
                for (i=files.begin(); i!=files.end(); ++i)
                        load_song_dir_recursive(*i, songset);
        } else
                load_one_song(filename, songset);
}

void Sound_Handler::import_music_class(Section* s, const songset_id songset)
{
        const char* filename;

        while ( s->get_next_string(songset_names.at(songset).c_str(), &filename) )
                load_song_dir_recursive(filename, songset);

        current_song[songset]=songs[songset].begin();

        if (songs[songset].empty())
                log(("Sound_Handler: Could not load any "+songset_names.at(songset)+" music or no "+
                                songset_names.at(songset)+" music configured!\n").c_str());
}

void Sound_Handler::read_config()
{
        Section* s;

        s=g_options.pull_section("music");
        disable_sound=s->get_bool("disable_sound",false);
        random_order=s->get_bool("random_order", true);
        if (disable_sound) return;

        import_music_class(s, INTRO);
        import_music_class(s, MENU);
        import_music_class(s, INGAME);

        s=g_options.pull_section("soundfx");
        //TODO: write loading of sound fx
        try {
        }
        catch(...) {
                critical_error("Sound_Handler: Unhandled exception while loading sound fx files");
        }
}

Mix_Music* Sound_Handler::get_mix_music(const songset_id songset)
{
        Mix_Music* s;
        int songnumber;

        if (disable_sound)  //just paranoia - we can't arrive here if this is true
                return NULL;
        if(songs[songset].empty())
                return NULL;

        if (random_order) {
                songnumber=rng.rand()%songs[songset].size();
                return songs[songset].at(songnumber);
        } else {
                s=*(current_song[songset]++);

                //++current_song[songset];
                if (current_song[songset]==songs[songset].end())
                        current_song[songset]=songs[songset].begin();

                return s;
        } //random order

        log("Sound_Handler: Undefined music class requested: %s\n", songset_names.at(songset).c_str());
        return NULL;
}

void Sound_Handler::start_music(const songset_id songset, int fadein_ms)
{
        Mix_Music* m;

        if (disable_sound) return;
        if (fadein_ms==0) fadein_ms=50;  //avoid clicks
        if (songset!=NONE) current_songset=songset;

        m=get_mix_music(current_songset);
        if (m)
                Mix_FadeInMusic(m, 1, fadein_ms);
        else
                log("Sound_Handler: No %s music found!\n", songset_names.at(current_songset).c_str());
}

void Sound_Handler::change_music(const songset_id songset, int fadeout_ms, int fadein_ms)
{
        stop_music(fadeout_ms);
        start_music(songset, fadein_ms);
}

void Sound_Handler::stop_music(int fadeout_ms)
{
        if (disable_sound) return;
        if (fadeout_ms==0) fadeout_ms=50;  //avoid clicks

        Mix_FadeOutMusic(fadeout_ms);

        while( Mix_FadingMusic()==MIX_FADING_OUT ); //wait until music has stopped
}

void Sound_Handler::music_finished_callback()
{
        assert(sound_handler); //yes, I'm paranoid

        //special case for splashscreen: there, only one song is ever played
        if (sound_handler->current_songset==Sound_Handler::INTRO) {
                SDL_Event* e=new SDL_Event();

                e->type=SDL_KEYDOWN;
                e->key.state=SDL_PRESSED;
                e->key.keysym.sym=SDLK_ESCAPE;
                SDL_PushEvent(e);
        } else
                sound_handler->change_music(NONE, 0, 0);

}
