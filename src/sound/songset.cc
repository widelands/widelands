/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "sound/songset.h"

#include "base/log.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "wlapplication_options.h"

class Song {

    public:

    Song(std::string filename) {
        songname = filename;
        enabled = true;
    }

    std::string songname;
    std::string title;
    bool enabled;
};

/// Prepare infrastructure for reading song files from disk and register the matching files
Songset::Songset(const std::string& dir, const std::string& basename) {
	assert(g_fs);

    std::vector<std::string> mp3_files = g_fs->get_sequential_files(dir, basename, "mp3");
    std::vector<std::string> ogg_files = g_fs->get_sequential_files(dir, basename, "ogg");

    if (basename == "ingame") {
        load_playlist();

        if (playlist_.empty()) {
            init_playlist(mp3_files);
            init_playlist(ogg_files);

            load_playlist();
        }
    }

    add_songs(mp3_files);
	add_songs(ogg_files);

}

/**
 * Initializes playlist_ from music files (all enabled by default)
 * \param files filenames that will be added
 */
void Songset::init_playlist(std::vector<std::string> files) {
    for (std::string f : files) {
        Song* song = new Song(f);
        playlist_.emplace(f, song);
    }
}

/**
 * Loads playlist data from config into playlist_
 */
void Songset::load_playlist() {
    try {
        Section sec = get_config_section("playlist");
        std::vector<Section::Value> values = sec.get_values();
        if (values.empty()) {
            for (Section::Value val : values) {
                std::string filename = val.get_name();
                bool value = val.get_bool();
                Song* song = playlist_[filename];
                if (song != nullptr) {
                    song->enabled = value;
                    song->songname = filename;
                    m_ = load_file(filename);
                    std::string title = Mix_GetMusicTitle(m_);
                    song->title = title;
                }
            }
        }
    } catch(WException& ex) {
       log_warn("Failed to load playlist");
    }
}

void Songset::add_songs(const std::vector<std::string>& files) {
    for (const std::string& filename : files) {
		assert(!g_fs->is_directory(filename));
		add_song(filename);
	}
}

/// Close and delete all songs to avoid memory leaks.
Songset::~Songset() {
	songs_.clear();

	if (m_ != nullptr) {
		Mix_FreeMusic(m_);
	}

	if (rwops_ != nullptr) {
		SDL_FreeRW(rwops_);
		fr_.close();
	}
}

/** Append a song to the end of the songset
 * \param filename  the song to append
 * \note The \ref current_song will unconditionally be set to the songset's
 * first song. If you do not want to disturb the (linear) playback order then
 * \ref register_song() all songs before you start playing
 */
void Songset::add_song(const std::string& filename) {
	songs_.push_back(filename);
	current_song_ = 0;
}

/**
 * Lets the playlist know if a song should be played or not
 */
bool Songset::is_song_enabled(std::string& filename) {
    return playlist_[filename]->enabled;
}

/**
 * Toggle wether to play or skip the given song for this songset
 */
void Songset::set_song_enabled(std::string& filename, bool on) {

}


/**
 * Gets info about all music in this songset (no audio data)
 * \return filename and title of all music in this songset
 */
std::vector<std::tuple<std::string,std::string>> Songset::get_songdata() {
    std::vector<std::tuple<std::string,std::string>> data;
    for (std::string filename : songs_) {
        m_ = load_file(filename);
        std::string title = Mix_GetMusicTitle(m_);
        data.push_back(make_tuple(filename, title));
    }
    return data;
}

/**
 * Get the audio data for the next song in the songset, or a random song
 * \param random an optional random number for picking the song
 * \return  a pointer to the chosen song; nullptr if none was found
 *          or an error occurred
 */
Mix_Music* Songset::get_song(uint32_t random) {
    std::string filename;

    if (songs_.empty()) {
        return nullptr;
    }

    if (random != 0) {
        // Exclude current_song from playing two times in a row
        current_song_ += 1 + random % (songs_.size() - 1);
        current_song_ = current_song_ % songs_.size();
    }

    filename = songs_.at(current_song_);

    // current_song is incremented after filename was chosen for two reasons:
    // 1. so that unshuffled playback starts at 0
    // 2. to prevent playing same song two times in a row immediately after shuffle is turned off
    if (++current_song_ >= songs_.size()) {
        current_song_ = 0; // wrap
    }

    m_ = load_file(filename);
    return m_;
}


/**
 * Loads a song file from disk
 * \param filename
 * \return music data
 */
Mix_Music* Songset::load_file(std::string filename) {

    // First, close the previous song and remove it from memory
    if (m_ != nullptr) {
        Mix_FreeMusic(m_);
        m_ = nullptr;
    }

    if (rwops_ != nullptr) {
        SDL_FreeRW(rwops_);
        rwops_ = nullptr;
        fr_.close();
    }

    // Then open the new song
    if (fr_.try_open(*g_fs, filename)) {
        rwops_ = SDL_RWFromMem(fr_.data(0), fr_.get_size());
        if (rwops_ == nullptr) {
            fr_.close();  // fr_ should be Open iff rwops_ != 0
            return nullptr;
        }
    } else {
        return nullptr;
    }

    if (rwops_ != nullptr) {
        m_ = Mix_LoadMUS_RW(rwops_, 0);
    }

    if (m_ != nullptr) {
        log_info("Songset: Loaded song \"%s\"\n", filename.c_str());
    } else {
        log_err("Songset: Loading song \"%s\" failed!\n", filename.c_str());
        log_err("Songset: %s\n", Mix_GetError());
    }

    return m_;
}
