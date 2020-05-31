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
#include <cstring>
#include <memory>

#include "base/log.h"
#include "base/wexception.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamread.h"
#include "logic/filesystem_constants.h"

// silence warnings triggered by curl.h
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")

// all cURL-related code is inspired by
// https://stackoverflow.com/questions/1636333/download-file-using-libcurl-in-c-c

void NetAddons::init() {
	if (curl_) {
		// already initialized
		return;
	}
	curl_ = curl_easy_init();
	if (!curl_) {
		throw wexception("Unable to initialize cURL");
	}
}

NetAddons::~NetAddons() {
	if (curl_) {
		curl_easy_cleanup(curl_);
		curl_ = nullptr;
	}
}

static size_t refresh_remotes_callback(char* received_data, size_t, const size_t char_count, std::string* out) {
	for (size_t i = 0; i < char_count; ++i) {
		(*out) += received_data[i];
	}
	return char_count;
}

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

	curl_easy_setopt(curl_, CURLOPT_URL, "https://raw.githubusercontent.com/Noordfrees/wl_addons_server/master/list");

	std::string output;
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &output);

	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &refresh_remotes_callback);

	const CURLcode res = curl_easy_perform(curl_);

	if (res != CURLE_OK) {
		throw wexception("cURL terminated with error code %d", res);
	}
	if (output.empty()) {
		throw wexception("cURL output is empty");
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
		return std::strtol(word.c_str(), nullptr, 10);
	};

	const size_t nr_addons = next_number(output);
	for (size_t i = 0; i < nr_addons; ++i) {
		AddOnInfo info;

		info.internal_name = next_word(output);

		const std::string descname = next_word(output);
		const std::string descr = next_word(output);
		info.descname = [descname]() { return descname; };
		info.description = [descr]() { return descr; };

		info.author = next_word(output);

		info.version = next_number(output);
		info.i18n_version = next_number(output);

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

		info.verified = next_word(output) == "verified";

		result_vector.push_back(info);
	}

	return result_vector;
}

// TODO(Nordfriese): Add-on downloading speed would benefit greatly from storing
// the files as ZIPs on the server. Similar for translation bundles. Perhaps
// someone would like to write code to uncompress a downloaded ZIP file some day…

void NetAddons::download_addon_file(const std::string& name, const std::string& output) {
	init();

	const std::string url = "https://raw.githubusercontent.com/Noordfrees/wl_addons_server/master/addons/" + name;
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

	std::FILE* out_file = std::fopen(output.c_str(), "wb");
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, std::FILE* stream) {
		return std::fwrite(ptr, size, nmemb, stream);
	});
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, out_file);

	const CURLcode res = curl_easy_perform(curl_);

	fclose(out_file);

	if (res != CURLE_OK) {
		throw wexception("cURL terminated with error code %d", res);
	}
}

std::string NetAddons::download_i18n(const std::string& name, const std::string& locale) {
	init();

	const std::string temp_dirname = kTempFileDir + g_fs->file_separator() + name + ".mo" + kTempFileExtension;
	g_fs->ensure_directory_exists(temp_dirname);

	const std::string relative_output = temp_dirname + g_fs->file_separator() + locale + ".mo" + kTempFileExtension;
	const std::string canonical_output = g_fs->canonicalize_name(g_fs->get_userdatadir() + "/" + relative_output);

	const std::string url = "https://raw.githubusercontent.com/Noordfrees/wl_addons_server/master/i18n/" + name + "/" + locale + ".mo";
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

	std::FILE* out_file = std::fopen(canonical_output.c_str(), "wb");
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, [](void* ptr, size_t size, size_t nmemb, std::FILE* stream) {
		return std::fwrite(ptr, size, nmemb, stream);
	});
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, out_file);

	const CURLcode res = curl_easy_perform(curl_);

	fclose(out_file);

	bool success = true;
	if (res == CURLE_OK) {
		// If the locale does not exist, we may get a valid file containing the text "404: Not Found".
		// So we open the file, read the first 14 characters, and check whether this is the case.
		// Another problem we can forget about when we have a real server…
		try {
			std::unique_ptr<StreamRead> checker(g_fs->open_stream_read(relative_output));
			if (!checker) {
				success = false;
				log("ERROR: Downloading add-on translation %s for %s to %s: Unable to open output file\n",
						locale.c_str(), name.c_str(), canonical_output.c_str());
			} else {
				char buffer[15];
				checker->data(&buffer, 14);
				buffer[14] = '\0';
				if (std::strcmp(buffer, "404: Not Found") == 0) {
					success = false;
					log("WARNING: Downloading add-on translation %s for %s to %s was skipped due to '404: Not Found'\n",
							locale.c_str(), name.c_str(), canonical_output.c_str());
				}
			}
		} catch (const std::exception& e) {
			success = false;
			log("ERROR: Downloading add-on translation %s for %s to %s: Invalid output file (%s)\n",
					locale.c_str(), name.c_str(), canonical_output.c_str(), e.what());
		}
	} else {
		success = false;
		log("ERROR: Downloading add-on translation %s for %s to %s: cURL returned error code %d\n",
				locale.c_str(), name.c_str(), canonical_output.c_str(), res);
	}

	return success ? canonical_output : "";
}

CLANG_DIAG_ON("-Wdisabled-macro-expansion")
