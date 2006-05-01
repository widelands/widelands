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

#include "constants.h"
#include "error.h"
#include "filesystem.h"
#include "journal.h"
#include "machdep.h"
#include "wexception.h"

/**
 * Write a char value to the recording file.
 *
 * \param v The character to be written
 *
 * \note Simple wrapper function to make stdio file access less painful
 * Will vanish when IO handling gets moved to C++ streams
 */
void Journal::write_record_char(char v)
{
	if (fwrite(&v, sizeof(v), 1, m_recordfile) != 1)
		throw wexception("Write of 1 byte to record failed.");
	fflush(m_recordfile);
}

/**
 * Read a char value from the playback file
 *
 * \return The char that was read
 *
 *\note Simple wrapper function to make stdio file access less painful
 * Will vanish when IO handling gets moved to C++ streams
 */
char Journal::read_record_char()
{
	char v;

	if (fread(&v, sizeof(v), 1, m_playbackfile) != 1)
		throw wexception("Read of 1 byte from record failed.");

	return v;
}

/**
 * Write an int value to the recording file.
 *
 * \param v The int to be written
 *
 * \note Not 64bit-safe!
 * \note Simple wrapper function to make stdio file access less painful
 * Will vanish when IO handling gets moved to C++ streams
 */
void Journal::write_record_int(int v)
{
	assert(m_recordfile);

	v = Little32(v);
	if (fwrite(&v, sizeof(v), 1, m_recordfile) != 1)
		throw wexception("Write of 4 bytes to record failed.");
	fflush(m_recordfile);
}

/**
 * Read an int value from the playback file.
 *
 * \return The int that was read
 * Simple wrapper function to make stdio file access less painful
 * Will vanish when IO handling gets moved to C++ streams
 */
int Journal::read_record_int()
{
	int v;
	assert(m_playbackfile);

	if (fread(&v, sizeof(v), 1, m_playbackfile) != 1)
		throw wexception("Read of 4 bytes from record failed.");

	return Little32(v);
}

/**
 * \note Simple wrapper function to make stdio file access less painful
 * Will vanish when IO handling gets moved to C++ streams
 */
void Journal::write_record_code(uchar code)
{
	write_record_char(code);
}

/**
 * \note Simple wrapper function to make stdio file access less painful
 * Will vanish when IO handling gets moved to C++ streams
 */
void Journal::read_record_code(uchar code)
{
	uchar filecode;

	filecode = read_record_char();

	if (filecode != code)
		throw wexception("%08lX: Bad code %02X during playback (%02X "
				"expected). Mismatching executable versions?",
				get_playback_offset()-1, filecode, code);
}

///Standard ctor
Journal::Journal():
		m_recordname(""), m_playbackname(""),
		m_recordfile(0), m_playbackfile(0),
		m_record(false), m_playback(false)
{
}

/**
 * Close any open journal files
 */
Journal::~Journal()
{
	stop_recording();
	stop_playback();
}

/**
 * Start recording events handed to us
 * \param filename File the events should be written to
 * \todo Error handling
 */
void Journal::start_recording(std::string filename)
{
	if (m_recordfile)
		assert(1==0);//TODO: barf

	if(filename.empty()) {
		assert(1==0); //TODO: barf
	}

	m_recordname=std::string(FS_CanonicalizeName2(filename));
	m_recordfile = fopen(m_recordname.c_str(), "wb");
	if (m_recordfile) {
		log("Recording into %s\n", m_recordname.c_str());
		write_record_int(RFC_MAGIC);
		m_record=true;
	} else {
		throw wexception("Failed to open record file %s",
		                 m_recordname.c_str());
	}
}

/**
 * Stop recording events
 * \todo error handling
 */
void Journal::stop_recording()
{
	if (m_record) {
		fclose(m_recordfile);
		m_recordfile=0;
		m_record=false;
	}
}

/**
 * Start playing back events
 * \param filename File to get events from
 * \todo Error handling
 */
void Journal::start_playback(std::string filename)
{
	if (m_playbackfile)
		assert(1==0);//TODO: barf

	if(filename.empty()) {
		assert(1==0); //TODO: barf
	}

	m_playbackname=std::string(FS_CanonicalizeName2(filename));
	m_playbackfile = fopen(m_playbackname.c_str(), "rb");
	if (m_playbackfile) {
		log("Playing back from %s\n", m_playbackname.c_str());
		if (read_record_int() != RFC_MAGIC)
			throw wexception("Playback file has wrong magic number");
		m_playback=true;
	} else {
		throw wexception("Failed to open playback file %s",
		                 m_playbackname.c_str());
	}
}

/**
 * Stop recording events
 * \todo error handling
 */
void Journal::stop_playback()
{
	if (m_playback) {
		fclose(m_playbackfile);
		m_playbackfile=0;
		m_playback=false;
	}
}

/**
 * Record an event into the playback file. This entails serializing the event
 * writing it out.
 *
 * \param e The event to be recorded
 * \todo do this only if recording is enabled
 * \todo error handling
 */
void Journal::record_event(SDL_Event *e)
{
	switch(e->type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		write_record_char(RFC_EVENT);
		write_record_char((e->type == SDL_KEYUP) ? RFC_KEYUP : RFC_KEYDOWN);
		write_record_int(e->key.keysym.sym);
		write_record_int(e->key.keysym.unicode);
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		write_record_char(RFC_EVENT);
		write_record_char((e->type == SDL_MOUSEBUTTONUP) ? RFC_MOUSEBUTTONUP : RFC_MOUSEBUTTONDOWN);
		write_record_char(e->button.button);
		break;

	case SDL_MOUSEMOTION:
		write_record_char(RFC_EVENT);
		write_record_char(RFC_MOUSEMOTION);
		write_record_int(e->motion.x);
		write_record_int(e->motion.y);
		write_record_int(e->motion.xrel);
		write_record_int(e->motion.yrel);
		break;

	case SDL_QUIT:
		write_record_char(RFC_EVENT);
		write_record_char(RFC_QUIT);
		break;
	default:
		// can't really do anything useful with this event
		break;
	}
}

/**
 * Get an event from the playback file. This entails creating an empty event with
 * sensible default values (not all parameters get recorded) and deserializing
 * the event record.
 *
 * \param e The event being returned
 * \todo do this only if playback is enabled
 * \todo error handling
 */
bool Journal::read_event(SDL_Event *e)
{
	uchar code = read_record_char();
	bool haveevent=false;

	if (code == RFC_EVENT)
	{
		code = read_record_char();

		switch(code) {
			case RFC_KEYDOWN:
			case RFC_KEYUP:
				e->type = (code == RFC_KEYUP) ?
						SDL_KEYUP : SDL_KEYDOWN;
				e->key.keysym.sym=
						(SDLKey)read_record_int();
				e->key.keysym.unicode=
						read_record_int();
				break;

			case RFC_MOUSEBUTTONDOWN:
			case RFC_MOUSEBUTTONUP:
				e->type = (code == RFC_MOUSEBUTTONUP) ?
				           SDL_MOUSEBUTTONUP :
					SDL_MOUSEBUTTONDOWN;
				e->button.button = read_record_char();
				break;

			case RFC_MOUSEMOTION:
				e->type = SDL_MOUSEMOTION;
				e->motion.x = read_record_int();
				e->motion.y = read_record_int();
				e->motion.xrel = read_record_int();
				e->motion.yrel = read_record_int();
				break;

			case RFC_QUIT:
				e->type = SDL_QUIT;
				break;

			default:
				throw wexception("%08lX: Unknown eent type %02X in playback.",
						get_playback_offset()-1, code);
		}

		haveevent = true;
	}
	else if (code == RFC_ENDEVENTS)
	{
		haveevent = false;
	}
	else
		throw wexception("%08lX: Bad code %02X in eent playback.",
				 get_playback_offset()-1, code);

	return haveevent;
}

/**
 * Do the right thing with timestamps.
 * All timestamps handled with \ref WLApplication::get_time() pass through here.
 * If necessary, they will be recorded. On playback, they will be modified to
 * show the recorded time instead of the current time.
 */
void Journal::timestamp_handler(Uint32 *stamp)
{
	if (m_record) {
		write_record_code(RFC_GETTIME);
		write_record_int(*stamp);
	}

	if (m_playback) {
		read_record_code(RFC_GETTIME);
		*stamp=read_record_int();
	}
}

/**
 * \todo document me
 */
void Journal::set_idle_mark()
{
	write_record_char(RFC_ENDEVENTS);
}
