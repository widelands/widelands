/*
 * Copyright (C) 2006-2008 by the Widelands Development Team
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

#ifndef JOURNAL_H
#define JOURNAL_H

#include "journal_exceptions.h"

#include <SDL_events.h>

#include <fstream>
#include <string>
#include <cstring>

/**
 * Journal encapsulates all operations that are necessary for recording and
 * playing back a session. On it's interface, it deals with SDL_Events that
 * might (or might not) be recorded/played back. Whether a recording / playback
 * is actually being performed is internal to Journal.
 *
 * \note If you are hacking this class, throw a Journalfile_exception only
 * and always if there is a nonrecoverable error and you have already dealt with
 * it.
 *
 * \todo The idea of writing enums into a file is bad: enums are int32_t and
 * int32_t varies in size (typ. 32/64bit). Our own codes only need 8bit, so we
 * force IO down to this value. The same happens with keyboard events at
 * 32 bits. Cutting off bits is not a good solution, but in this case it'll do
 * until a better way comes along.
 */
struct Journal {
	/// change this and I will ensure your death will be a most unpleasant one
	static uint32_t const RFC_MAGIC = 0x0ACAD100;

	/**
	 * Record file codes
	 * It should be possible to use record files across different platforms.
	 * However, 64 bit platforms are currently not supported.
	 */
	enum rfccode {
		RFC_GETTIME         = 0x01,
		RFC_EVENT           = 0x02,
		RFC_ENDEVENTS       = 0x03,

		RFC_KEYDOWN         = 0x10,
		RFC_KEYUP           = 0x11,
		RFC_MOUSEBUTTONDOWN = 0x12,
		RFC_MOUSEBUTTONUP   = 0x13,
		RFC_MOUSEMOTION     = 0x14,
		RFC_QUIT            = 0x15,
		RFC_INVALID         = 0xff
	};

public:
	Journal();
	~Journal();

	void start_recording(std::string const & filename = "widelands.jnl");
	void stop_recording();
	///True if events are being recorded
	bool is_recording() {return m_record;}

	void start_playback (std::string const & filename = "widelands.jnl");
	void stop_playback();
	///True if events are being played back
	bool is_playingback() {return m_playback;}

	void record_event(SDL_Event *e);
	bool read_event(SDL_Event *e);

	void timestamp_handler(uint32_t * stamp);
	void set_idle_mark();

protected:
	/**
	 * Returns the position in the playback file
	 * \return byte offset into the playback file, used with file reading
	 */
	int32_t get_playback_offset() {return m_playbackstream.tellg();}

	void write(int8_t   v);
	void write(uint8_t  v);
	void write(int16_t  v);
	void write(uint16_t v);
	void write(int32_t  v);
	void write(uint32_t v);
	void write(SDLKey v);
	void write(SDLMod v);

	void read(int8_t   &v);
	void read(uint8_t  &v);
	void read(int16_t  &v);
	void read(uint16_t &v);
	void read(int32_t  &v);
	void read(uint32_t &v);
	void read(SDLKey &v);
	void read(SDLMod &v);
	void ensure_code(uint8_t code);

	///The recording file's name.
	///\note This does \e not go through the layered filesystem on purpose!
	std::string m_recordname;

	///The playback file's name.
	///\note This does \e not go through the layered filesystem on purpose!
	std::string m_playbackname;

	///The file events are being recorded to
	std::ofstream m_recordstream;

	///The file events are being played back from
	std::ifstream m_playbackstream;

	///True if events are being recorded
	bool m_record;

	///True if events are being played back
	bool m_playback;
};

#endif
