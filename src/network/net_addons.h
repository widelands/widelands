/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#ifndef WL_NETWORK_NET_ADDONS_H
#define WL_NETWORK_NET_ADDONS_H

#define CURL_STATICLIB

#include <curl/curl.h>
#include <set>

#include "logic/addons.h"

namespace AddOns {

// The add-on related networking functions defined here use the CURL lib.
// Pro: I created a functional dummy server with no knowledge of the metaserver backend ;)
// Con: Additional dependency – this is the only place in our code where libcurl is used

struct NetAddons {
	NetAddons() : initialized_(false), curl_(nullptr), client_socket_(0) {
	}
	~NetAddons();

	// Fetch the list of all available add-ons from the server
	std::vector<AddOnInfo> refresh_remotes();

	using CallbackFn = std::function<void(const std::string&, long)>;

	// Downloads the add-on with the given name (e.g. "cool_feature.wad")
	// from the server and downloads it to the given canonical location.
	void download_addon(const std::string& name, const std::string& save_as, const CallbackFn& progress);

	// Requests the MO files for the given add-on (cool_feature.wad) from the server and
	// downloads them into the given temporary location (e.g. ~/.widelands/temp/some_dir).
	// The filename of the created MO files is guaranteed to be in the format
	// "nds.mo.tmp" (where 'nds' is the language's abbreviation).
	void download_i18n(const std::string& addon, const std::string& directory);

	// Download the given screenshot for the given add-on
	std::string download_screenshot(const std::string& addon, const std::string& screenie);

private:
	friend struct CrashGuard;

	// Open the connection if it was not open yet; throws an error if this fails
	void init();

	// Set the URL (whitespace-safe) and adjust the timeout values.
	void set_url_and_timeout(std::string);

	// Read a '\n'-terminated string from the socket. The terminator is not part of the result.
	std::string read_line();

	bool initialized_;
	CURL* curl_;
	int client_socket_;
};

}  // namespace AddOns

#endif  // end of include guard: WL_NETWORK_NET_ADDONS_H
