/*
* Copyright (C) 2010 The Widelands Development Team
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __WIDELANDS_CLIENT_H__
#define __WIDELANDS_CLIENT_H__

#include <string>
#include <stdint.h>
#include <vector>
#include "protocol.h"
#include "widelands_server.h"
#include "log.h"

#define SUPPORT_B16_PROTOCOL(p) (p != 0 and p->support_build16_proto())

class WidelandsClient {
public:
	WidelandsClient(std::string name, int ggznum):
		connection_failed(false),
		desync(false),
		hard_error_count(0),
		soft_error_count(0),
		m_name(name),
		m_ggz_number(ggznum),
		m_wl_number(-1),
		m_reported(false),
		m_proto_maj(0),
		m_proto_min(0),
		m_version(),
		m_build(),
		m_playertype(playertype_null)
	{}

	int ggz_number() const { return m_ggz_number; }
	int wl_player_number() const { return m_wl_number; }
	std::string name() const { return m_name; }
	bool reported_game() const {return m_reported; }
	bool support_build16_proto() const { return m_proto_maj > 0; }
	std::string version() const { return m_version; }
	std::string build() const { return m_build; }

	void set_ggz_number(int num) { 
		wllog
			(DL_DUMP, "Set ggz number for \"%s\": %i",
			 name().c_str(), num);
		m_ggz_number = num;
	}

	void set_wl_number(int num) { 
		wllog
			(DL_DUMP, "Set wl number for \"%s\": %i",
			 name().c_str(), num);
		m_wl_number = num;
	}

	void set_reported() {m_reported = true; }
	void set_proto(int maj, int min) {
		wllog
			(DL_DUMP, "Set proto version for \"%s\": %i, %i",
			 name().c_str(), maj, min);
		m_proto_maj = maj;
		m_proto_min = min;
	}

	void set_version (std::string ver, std::string build) {
		wllog
			(DL_DUMP, "Set wl version for \"%s\": \"%s\", \"%s\"",
			 name().c_str(), ver.c_str(), build.c_str());
		m_version = ver;
		m_build = build;
	}

	void set_type(WLGGZPlayerType t) {
		 m_playertype = t;
	}
	
	void set_build16_proto(int maj, int min) {
		m_proto_maj = maj;
		m_proto_min = min;
	}

	bool connection_failed, desync;
	int hard_error_count, soft_error_count;

private:
	std::string m_name;
	int m_ggz_number;
	int m_wl_number;
	bool m_reported;
	int m_proto_maj, m_proto_min;
	std::string m_version, m_build;
	WLGGZPlayerType m_playertype;
};

#endif