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

#include "network/net_addons.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>

#include <arpa/inet.h>
#include <boost/format.hpp>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "base/i18n.h"
#include "base/md5.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/filesystem_constants.h"
#include "wlapplication_options.h"

namespace AddOns {

// silence warnings triggered by curl.h
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")

// all CURL-related code is inspired by
// https://stackoverflow.com/questions/1636333/download-file-using-libcurl-in-c-c
//
// all other networking-related code in this file is inspired by
// https://www.thecrazyprogrammer.com/2017/06/socket-programming.html

void NetAddons::init() {
	if (initialized_) {
		// already initialized
		return;
	}

	if (!curl_) {
		curl_ = curl_easy_init();
	}
	if (!curl_) {
		throw wexception("Unable to initialize CURL");
	}
#ifdef __MINGW32__
		// mingw with static curl causes some trouble
		curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0);
#endif

	if ((client_socket_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		throw wexception("Unable to create socket");
	}
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(get_config_int("addon_server_port", 7399));
	server.sin_addr.s_addr = inet_addr(get_config_string("addon_server_ip", "127.0.0.1" /* NOCOM */).c_str());
	if (connect(client_socket_, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
		throw wexception("Unable to connect to the server");
	}

	initialized_ = true;
}

NetAddons::~NetAddons() {
	if (curl_) {
		curl_easy_cleanup(curl_);
		curl_ = nullptr;
	}
}

void NetAddons::set_url_and_timeout(std::string url) {
	size_t pos = 0;
	while ((pos = url.find(' ')) != std::string::npos) {
		url.replace(pos, 1, "%20");
	}
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

	// Times are in seconds
	curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30);
}

std::string NetAddons::read_line() {
	std::string line;
	char c;
	int n;
	for (;;) {
		n = read(client_socket_, &c, 1);
		if (n != 1 || c == '\n') {
			break;
		}
		line += c;
	}
	return line;
}

static inline std::string get_addons_repo_name() {
	return std::string("https://raw.githubusercontent.com/") +
	       get_config_string("addon_repo", "widelands/wl_addons_server/master") + "/";
}

// A crash guard is there to ensure that the socket connection will be reset
// in case an unexpected problem occurs, to prevent subsequent actions from
// reading or writing random leftover bytes. Create it before doing some
// networking stuff and call `ok()` after everything has gone well.
struct CrashGuard {
	explicit CrashGuard(NetAddons& n) : net_(n) {
		assert(net_.initialized_);
		net_.initialized_ = false;
	}
	void ok() {
		assert(!net_.initialized_);
		net_.initialized_ = true;
	}

private:
	NetAddons& net_;
};

constexpr unsigned kCurrentListVersion = 3;
std::vector<AddOnInfo> NetAddons::refresh_remotes() {
	init();
	CrashGuard guard(*this);

	{
		const char* data_to_send = "CMD_LIST\n";
		write(client_socket_, data_to_send, strlen(data_to_send));
	}

	const long nr_addons = std::stol(read_line().c_str());
	std::vector<AddOnInfo> result_vector(nr_addons);
	for (long i = 0; i < nr_addons; ++i) {
		result_vector[i].internal_name = read_line();
	}

	for (long i = 0; i < nr_addons; ++i) {
		AddOnInfo& a = result_vector[i];
		{
			std::string send = "CMD_INFO ";
			send += std::to_string(kCurrentListVersion);
			send += ' ';
			send += a.internal_name;
			send += ' ';
			send += i18n::get_locale();
			send += '\n';
			write(client_socket_, send.c_str(), send.size());
		}

		a.unlocalized_descname = read_line();
		const std::string localized_descname = read_line();
		a.descname = [localized_descname]() { return localized_descname; };
		a.unlocalized_description = read_line();
		const std::string localized_description = read_line();
		a.description = [localized_description]() { return localized_description; };
		a.unlocalized_author = read_line();
		const std::string localized_author = read_line();
		a.author = [localized_author]() { return localized_author; };
		a.upload_username = read_line();
		a.version = string_to_version(read_line());
		a.i18n_version = std::stol(read_line());
		a.category = get_category(read_line());

		std::string req = read_line();
		for (; !req.empty();) {
			size_t pos = req.find(',');
			if (pos < req.size()) {
				a.requirements.push_back(req.substr(0, pos));
				req = req.substr(pos + 1);
			} else {
				a.requirements.push_back(req);
				break;
			}
		}

		for (int j = std::stoi(read_line()); j > 0; --j) {
			const std::string s1 = read_line();
			const std::string s2 = read_line();
			a.screenshots[s1] = s2;
		}
		a.total_file_size = std::stol(read_line());
		a.upload_timestamp = std::stol(read_line());
		a.download_count = std::stol(read_line());
		for (int j = 0; j < kMaxRating; ++j) {
			a.votes[j] = std::stol(read_line());
		}

		const int comments = std::stoi(read_line());
		a.user_comments.resize(comments);
		for (int j = 0; j < comments; ++j) {
			a.user_comments[j].username = read_line();
			a.user_comments[j].timestamp = std::stol(read_line());
			a.user_comments[j].version = string_to_version(read_line());
			int newlines = std::stoi(read_line());
			a.user_comments[j].message = read_line();
			for (; newlines > 0; --newlines) {
				a.user_comments[j].message += '\n';
				a.user_comments[j].message += read_line();
			}
		}
		a.verified = read_line() == "verified";
	}

	guard.ok();
	return result_vector;
}

void NetAddons::download_addon(const std::string& name, const std::string& save_as, const CallbackFn& progress) {
	init();
	CrashGuard guard(*this);
	{
		std::string send = "CMD_DOWNLOAD ";
		send += name;
		send += '\n';
		write(client_socket_, send.c_str(), send.size());
	}
	g_fs->ensure_directory_exists(save_as);

	const long nr_dirs = std::stol(read_line());
	std::unique_ptr<std::string[]> dirnames(new std::string[nr_dirs]);
	for (long i = 0; i < nr_dirs; ++i) {
		dirnames[i] = read_line();
		g_fs->ensure_directory_exists(save_as + FileSystem::file_separator() + dirnames[i]);
	}
	long progress_state = 0;
	for (long i = -1 /* top-level directory is not counted */; i < nr_dirs; ++i) {
		for (long j = std::stol(read_line()); j > 0; --j) {
			const std::string filename = read_line();
			const long length = std::stol(read_line());

			std::string relative_path;
			if (i >= 0) {
				relative_path += dirnames[i];
				relative_path += FileSystem::file_separator();
			}
			relative_path += filename;
			std::string out = save_as;
			out += FileSystem::file_separator();
			out += relative_path;

			FileWrite fw;
			std::unique_ptr<char[]> buffer(new char[length]);
			long nr_bytes_read = 0;
			do {
				progress(relative_path, progress_state);
				long l = read(client_socket_, buffer.get(), length - nr_bytes_read);
				if (l < 1) {
					throw wexception("Connection interrupted or server crashed");
				}
				nr_bytes_read += l;
				progress_state += l;
				fw.data(buffer.get(), l);
			} while (nr_bytes_read < length);
			fw.write(*g_fs, out);
		}
	}
	guard.ok();
}

/* static void check_downloaded_file(const std::string& path, const std::string& checksum) {
	try {
		// Our md5 implementation is not well documented, so I am doing this
		// as it is done in GameClient::handle_new_file and hope it works…
		FileRead fr;
		fr.open(*g_fs, path);
		const size_t bytes = fr.get_size();
		std::unique_ptr<char[]> complete(new char[bytes]);
		fr.data_complete(complete.get(), bytes);
		SimpleMD5Checksum md5sum;
		md5sum.data(complete.get(), bytes);
		md5sum.finish_checksum();
		const std::string md5 = md5sum.get_checksum().str();
		if (checksum != md5) {
			throw wexception("Downloaded file '%s': Checksum mismatch, found %s, expected %s",
			                 path.c_str(), md5.c_str(), checksum.c_str());
		}
	} catch (const std::exception& e) {
		throw wexception(
		   "Downloaded file '%s': Unable to check output file: %s", path.c_str(), e.what());
	}
}

void NetAddons::download_addon_file(const std::string& name,
                                    const std::string& checksum,
                                    const std::string& output) {
	init();

	set_url_and_timeout(get_addons_repo_name() + "addons/" + name);

	FileWrite fw;
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &download_addon_file_callback);
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &fw);

	const CURLcode res = curl_easy_perform(curl_);

	fw.write(*g_fs, output);

	if (res != CURLE_OK) {
		throw wexception("%s: CURL terminated with error code %d", name.c_str(), res);
	}
	check_downloaded_file(output, checksum);
}

std::string NetAddons::download_i18n(const std::string& name,
                                     const std::string& checksum,
                                     const std::string& locale) {
	init();

	const std::string temp_dirname =
	   kTempFileDir + FileSystem::file_separator() + name + ".mo" + kTempFileExtension;
	g_fs->ensure_directory_exists(temp_dirname);

	const std::string output =
	   temp_dirname + FileSystem::file_separator() + locale + kTempFileExtension;

	set_url_and_timeout(get_addons_repo_name() + "i18n/" + name + "/" + locale);

	FileWrite fw;
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &download_addon_file_callback);
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &fw);

	const CURLcode res = curl_easy_perform(curl_);

	fw.write(*g_fs, output);

	if (res != CURLE_OK) {
		throw wexception(
		   "[%s / %s] CURL terminated with error code %d\n", name.c_str(), locale.c_str(), res);
	}

	check_downloaded_file(output, checksum);
	return output;
} */

static size_t curl_download_callback(char* data, size_t, const size_t char_count, FileWrite* fw) {
	fw->data(data, char_count);
	return char_count;
}

std::string NetAddons::download_screenshot(const std::string& name, const std::string& screenie) {
	init();

	std::string temp_dirname =
	   kTempFileDir + FileSystem::file_separator() + name + ".screenshots" + kTempFileExtension;
	g_fs->ensure_directory_exists(temp_dirname);

	const std::string output = temp_dirname + FileSystem::file_separator() + screenie;

	set_url_and_timeout(get_addons_repo_name() + "screenshots/" + name + "/" + screenie);

	FileWrite fw;
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &curl_download_callback);
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &fw);

	const CURLcode res = curl_easy_perform(curl_);

	fw.write(*g_fs, output);

	return res == CURLE_OK ? output : "";
}

CLANG_DIAG_ON("-Wdisabled-macro-expansion")

}  // namespace AddOns
