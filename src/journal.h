/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#include <SDL_events.h>
#include <string>

/**
 * Journal encapsulates all operations that are necessary for recording and
 * playing back a session. On it's interface, it deals with SDL_Events that
 * might (or might not) be recorded/played back. Whether a recording / playback
 * is actually being performed is internal to Journal.
 */
class Journal {
public:
	Journal();
	~Journal();

	void start_recording(std::string filename="");
	void stop_recording();

	void start_playback(std::string filename="");
	void stop_playback();

	void record_event(SDL_Event *e);
	bool read_event(SDL_Event *e);

	void timestamp_handler(Uint32 *stamp);
	void set_idle_mark();

protected:
	/**
	 * Returns the position in the playback file
	 * \return byte offset into the playback file, used with file reading
	 */
	const long get_playback_offset() {return ftell(m_playbackfile);}

	void write_record_char(char v);
	char read_record_char();
	void write_record_int(int v);
	int read_record_int();
	void write_record_code(uchar code);
	void read_record_code(uchar code);

	///The recording file's name.
	///\note This does \e not go through the layered filesystem on purpose!
	std::string m_recordname;

	///The playback file's name.
	///\note This does \e not go through the layered filesystem on purpose!
	std::string m_playbackname;

	///The file events are being recorded to (or NULL)
	FILE *m_recordfile;

	///The file events are being played back from (or NULL)
	FILE *m_playbackfile;

	///True if events are being recorded
	bool m_record;

	///True if events are being played back
	bool m_playback;
};

#endif
