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

#ifndef WLAPPLICATION_H
#define WLAPPLICATION_H

/// A macro to make i18n more readable
#define _( str ) WLApplication::translate( str )

/**
 * Note:There is no need for a destructor, just make sure you call
 * \ref shutdown().
 *
 * \todo Document that record/playback files are journals
 */
class WLApplication {
public:
	static WLApplication *get(int argc=0, char **argv=0);

	void run();
	bool init();
	void shutdown();

	//Locale handling
	static const char* translate( const char* str ) {return gettext( str );}
	void grab_textdomain( const char* );
	void release_textdomain();
	void set_locale( const char* = 0);
	std::string get_locale() {return m_locale;}

	//Record/Playback file handling
	bool get_playback() {return m_playback;}
	int get_playback_offset();
	bool get_record() {return m_record;}
	FILE *get_rec_file() {return m_frecord;}
	FILE *get_play_file() {return m_fplayback;}

protected:
	WLApplication(int argc, char **argv);

	bool init_settings();
	void shutdown_settings();

	bool init_hardware();
	void shutdown_hardware();

	bool init_recordplaybackfile();
	void shutdown_recordplaybackfile();

	bool parse_command_line();
	void show_usage();

	std::string m_locale;
	std::vector<std::string> m_textdomains;

	bool m_playback, m_record;
	char m_recordname[256], m_playbackname[256];
	FILE *m_frecord, *m_fplayback;

	int m_argc;
	char **m_argv;

private:
	static WLApplication *the_singleton;
};

#endif
