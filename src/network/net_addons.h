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

#include <set>

#include "logic/addons.h"

namespace AddOns {

struct NetAddons {
	NetAddons() : initialized_(false), network_active_(false), client_socket_(0), is_admin_(false) {
	}
	~NetAddons();

	bool is_admin() const {
		return is_admin_;
	}

	// Fetch the list of all available add-ons from the server
	std::vector<std::string> refresh_remotes(bool all);
	AddOnInfo fetch_one_remote(const std::string& name);

	using CallbackFn = std::function<void(const std::string&, int64_t)>;

	// Downloads the add-on with the given name (e.g. "cool_feature.wad")
	// from the server and downloads it to the given canonical location.
	void download_addon(const std::string& name, const std::string& save_as, const CallbackFn&);

	// Requests the MO files for the given add-on (cool_feature.wad) from the server and
	// downloads them into the given temporary location (e.g. ~/.widelands/temp/some_dir).
	// The filename of the created MO files is guaranteed to be in the format
	// "nds.mo.tmp" (where 'nds' is the language's abbreviation).
	void download_i18n(const std::string& addon,
	                   const std::string& directory,
	                   const CallbackFn& progress,
	                   const CallbackFn& init_fn);

	// Download the given screenshot for the given add-on
	std::string download_screenshot(const std::string& addon, const std::string& screenie);

	// How the user voted the add-on (1-10). Returns 0 for not voted, <0 for access denied.
	int get_vote(const std::string& addon);
	void vote(const std::string& addon, unsigned vote);

	// Write a new comment or edit an existing one. If `index_to_edit` is `nullptr`,
	// a new comment will be written. An empty message causes the comment to be deleted.
	void comment(const AddOnInfo& addon, const std::string& message, const size_t* index_to_edit);

	void
	upload_addon(const std::string& addon, const CallbackFn& progress, const CallbackFn& init_fn);
	void upload_screenshot(const std::string& addon,
	                       const std::string& image,
	                       const std::string& description);

	enum class AdminAction { kVerify, kQuality, kSyncSafe, kSetupTx, kDelete };
	void admin_action(AdminAction, const AddOnInfo& addon, const std::string& value);

	void contact(const std::string& enquiry);

	void set_login(const std::string& username, const std::string& password);

private:
	friend struct CrashGuard;

	// Open the connection if it was not open yet; throws an error if this fails
	void init(std::string username = std::string(), std::string password = std::string());
	void quit_connection();

	// Read a '\n'-terminated string from the socket. The terminator is not part of the result.
	std::string read_line();
	void read_file(int64_t length, const std::string& out);
	void check_endofstream();
	void write_to_server(const std::string&);
	void write_to_server(const char*, size_t);

	std::string last_username_, last_password_;
	bool initialized_;
	bool network_active_;
	int client_socket_;
	bool is_admin_;
};

}  // namespace AddOns

#endif  // end of include guard: WL_NETWORK_NET_ADDONS_H
