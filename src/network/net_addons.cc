/*
 * Copyright (C) 2020-2020 by the Widelands Development Team
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

#include <boost/format.hpp>

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

void NetAddons::init() {
	if (curl_) {
		// already initialized
		return;
	}
	curl_ = curl_easy_init();

#ifdef __MINGW32__
	// mingw with static curl causes some trouble
	curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0);
#endif

	if (!curl_) {
		throw wexception("Unable to initialize CURL");
	}
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

static size_t
refresh_remotes_callback(char* received_data, size_t, const size_t char_count, std::string* out) {
	for (size_t i = 0; i < char_count; ++i) {
		(*out) += received_data[i];
	}
	return char_count;
}

static inline std::string get_addons_server_name() {
	return std::string("https://raw.githubusercontent.com/") +
	       get_config_string("addon_server", "widelands/wl_addons_server/master") + "/";
}

constexpr unsigned kCurrentListVersion = 2;
std::vector<AddOnInfo> NetAddons::refresh_remotes() {
	// TODO(Nordfriese): This connects to my personal dummy add-ons repo for demonstration.
	// A GitHub repo is NOT SUITED as an add-ons server because the list of add-ons needs
	// to be maintained by hand there which is exceedlingly fragile and messy.
	// Not to mention that non-devs cannot upload stuff to the repo.
	//
	// Also, we could theoretically tell the server which language we are speaking,
	// so the server would send localized add-on names and descriptions.
	// And we would not need to store a list of all files contained in every add-on
	// in the global catalogue. Both is not possible with such a dummy server.

	init();

	std::vector<AddOnInfo> result_vector;

	// For backwards compatibility, the server keeps multiple versions of the list
	set_url_and_timeout(get_addons_server_name() + "list_" + std::to_string(kCurrentListVersion));

	std::string output;
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &output);

	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &refresh_remotes_callback);

	const CURLcode res = curl_easy_perform(curl_);

	if (res != CURLE_OK) {
		throw wexception("CURL terminated with error code %d", res);
	}
	if (output.empty()) {
		throw wexception("CURL output is empty");
	}

	// We now have a list of the stuff we are interested in.
	// TODO(Nordfriese): The list uses an ugly dummy format designed to be manually
	// moddable (more or less). We want a real (compact) binary format when we have
	// a real server. The following ugly helper code can then also be deleted.

	auto next_word = [](std::string& str) {
		const size_t l = str.find('\n');
		std::string result = str.substr(0, l);
		str = str.substr(l + 1);
		return result;
	};
	auto next_number = [next_word](std::string& str) {
		const std::string word = next_word(str);
		return std::strtoll(word.c_str(), nullptr, 10);
	};

	const uint16_t list_version = next_number(output);
	if (list_version != kCurrentListVersion) {
		throw wexception("List version mismatch! Found version %u, supported version is %u",
		                 list_version, kCurrentListVersion);
	}

	const size_t nr_addons = next_number(output);
	for (size_t i = 0; i < nr_addons; ++i) {
		AddOnInfo info;

		info.internal_name = next_word(output);

		const std::string descname = next_word(output);
		const std::string descr = next_word(output);
		const std::string author = next_word(output);
		info.descname = [descname]() { return descname; };
		info.description = [descr]() { return descr; };
		info.author = [author]() { return author; };
		info.upload_username = next_word(output);

		info.upload_timestamp = next_number(output);
		info.download_count = next_number(output);
		for (uint8_t vote = 0; vote < kMaxRating; ++vote) {
			info.votes[vote] = next_number(output);
		}

		for (size_t comments = next_number(output); comments; --comments) {
			const std::string name = next_word(output);
			const std::string msg = next_word(output);
			const std::string v = next_word(output);
			const uint32_t t = next_number(output);
			info.user_comments.push_back(AddOnComment{name, msg, string_to_version(v), t});
		}

		info.version = string_to_version(next_word(output));
		info.i18n_version = next_number(output);
		info.total_file_size = next_number(output);

		info.category = get_category(next_word(output));

		for (size_t req = next_number(output); req; --req) {
			info.requirements.push_back(next_word(output));
		}

		for (size_t dirs = next_number(output); dirs; --dirs) {
			info.file_list.directories.push_back(next_word(output));
		}
		for (size_t files = next_number(output); files; --files) {
			info.file_list.files.push_back(next_word(output));
		}
		for (size_t locales = next_number(output); locales; --locales) {
			info.file_list.locales.push_back(next_word(output));
		}
		for (size_t sums = next_number(output); sums; --sums) {
			info.file_list.checksums.push_back(next_word(output));
		}
		if (info.file_list.checksums.size() !=
		    info.file_list.files.size() + info.file_list.locales.size()) {
			throw wexception("Found %u files and %u locales, but %u checksums",
			                 static_cast<unsigned>(info.file_list.files.size()),
			                 static_cast<unsigned>(info.file_list.locales.size()),
			                 static_cast<unsigned>(info.file_list.checksums.size()));
		}
		for (size_t screenies = next_number(output); screenies; --screenies) {
			const std::string s_name = next_word(output);
			const std::string s_descr = next_word(output);
			info.screenshots[s_name] = s_descr;
		}

		info.verified = next_word(output) == "verified";

		result_vector.push_back(info);
	}

	return result_vector;
}

static void check_downloaded_file(const std::string& path, const std::string& checksum) {
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

// TODO(Nordfriese): Add-on downloading speed would benefit greatly from storing
// the files as ZIPs on the server. Similar for translation bundles. Perhaps
// someone would like to write code to uncompress a downloaded ZIP file some day…

static size_t
download_addon_file_callback(char* data, size_t, const size_t char_count, FileWrite* fw) {
	fw->data(data, char_count);
	return char_count;
}

void NetAddons::download_addon_file(const std::string& name,
                                    const std::string& checksum,
                                    const std::string& output) {
	init();

	set_url_and_timeout(get_addons_server_name() + "addons/" + name);

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

	set_url_and_timeout(get_addons_server_name() + "i18n/" + name + "/" + locale);

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
}

std::string NetAddons::download_screenshot(const std::string& name, const std::string& screenie) {
	init();

	std::string temp_dirname =
	   kTempFileDir + FileSystem::file_separator() + name + ".screenshots" + kTempFileExtension;
	g_fs->ensure_directory_exists(temp_dirname);

	const std::string output = temp_dirname + FileSystem::file_separator() + screenie;

	set_url_and_timeout(get_addons_server_name() + "screenshots/" + name + "/" + screenie);

	FileWrite fw;
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &download_addon_file_callback);
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &fw);

	const CURLcode res = curl_easy_perform(curl_);

	fw.write(*g_fs, output);

	return res == CURLE_OK ? output : "";
}

CLANG_DIAG_ON("-Wdisabled-macro-expansion")

}  // namespace AddOns
