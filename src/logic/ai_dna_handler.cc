/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "logic/ai_dna_handler.h"

#include <cstring>
#include <ctime>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "helper.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "io/filesystem/filesystem.h"
#include "logic/constants.h"
#include "wlapplication.h"
#include "wui/interactive_base.h"

namespace Widelands {

constexpr double kAiDnaKeepAroundTime = 4 * 7 * 24 * 60 * 60;

AiDnaHandler::AiDnaHandler() {
	g_fs->ensure_directory_exists(get_base_dir());
	fs_type_ = FileSystem::DIR;

	FilenameSet ai_files;

	//// Getting rid of old ai dump files in AI directory - does not work yet
	//ai_files = filter(g_fs->list_directory(get_base_dir()),
	            //[](const std::string& fn) { return boost::ends_with(fn, AI_SUFFIX) ||
					//boost::contains(fn, "ai_player");});

	//for (auto file : ai_files) {
		//tm tfile;

		//if (!extract_day(file, &tfile)) {
			//continue;
		//}
		//if (std::difftime(time(nullptr), mktime(&tfile)) < kAiDnaKeepAroundTime) {
			//continue;
		//}
		//printf ("AI file to remove: %s (Not now)\n", file.c_str());
	//}

}

//// Extracts the creation date from 'path' which is expected to
//// match "YYYY-MM-DD*". Returns false if no date could be extracted.
//bool AiDnaHandler::extract_day(const std::string& filename, tm* tfile) {
	//memset(tfile, 0, sizeof(tm));
	//long day, month, year;
	//if (!to_long(filename.substr(8, 2), &day)) {
		//return false;
	//}
	//if (!to_long(filename.substr(5, 2), &month)) {
		//return false;
	//}
	//if (!to_long(filename.substr(0, 4), &year)) {
		//return false;
	//}
	//tfile->tm_mday = day;
	//tfile->tm_mon = month - 1;
	//tfile->tm_year = year - 1900;
	//return tfile;
//}

// this reads AI file for particular slot (position) and populate numbers into passed vectors
void AiDnaHandler::fetch_dna(std::vector<int16_t>& military_numbers,
                             std::vector<int8_t>& input_weights,
                             std::vector<int8_t>& input_func,
                             std::vector<uint32_t>& f_neurons,
                             uint8_t slot) {

	// AI files are in range 1-4
	assert(slot > 0 && slot < 5);

	std::string full_filename = get_base_dir() + g_fs->file_separator() + "ai_input_" + std::to_string(static_cast<int16_t>(slot)) + "." + AI_SUFFIX;

	Profile prof;
	prof.read(full_filename.c_str(), nullptr, *g_fs);
	Section& mn = prof.get_safe_section("magic_numbers");
	for (uint16_t i = 0; i < military_numbers.size(); i++) {
		int32_t value = mn.get_int(std::to_string(static_cast<int32_t>(i)).c_str());
		if (value < -100 || value > 100) {
			throw wexception("Out of range AI data in magic_numbers section: %d\n", i);
		}
		military_numbers[i] = static_cast<int16_t>(value);
	}

	Section& nv = prof.get_safe_section("neuron_values");
	for (uint16_t i = 0; i < input_weights.size(); i++) {
		int32_t value = nv.get_int(std::to_string(static_cast<int32_t>(i)).c_str());
		if (value < -100 || value > 100) {
			throw wexception("Out of range  AI data in neuron_values section: %d\n", i);
		}
		input_weights[i] = static_cast<int8_t>(value);
	}

	Section& nf = prof.get_safe_section("neuron_functions");
	for (uint16_t i = 0; i < input_func.size(); i++) {
		int32_t value = nf.get_int(std::to_string(static_cast<int32_t>(i)).c_str());
		if (value < 0 || value > 3) {
			throw wexception("Out of range  AI data in neuron_functions section: %d\n", i);
		}
		input_func[i] = static_cast<int8_t>(value);
	}

	Section& fn = prof.get_safe_section("fneurons");
	for (uint16_t i = 0; i < f_neurons.size(); i++) {
		uint32_t value = fn.get_natural(std::to_string(static_cast<int32_t>(i)).c_str());

		f_neurons[i] = value;
	}
}

//this generate new file with AI data in ai folder
void AiDnaHandler::dump_output(Widelands::Player::AiPersistentState* pd, uint8_t pn) {

	time_t t;
	time(&t);
	struct tm* currenttime = localtime(&t);

	std::string timestamp = str(boost::format("%d-%02d-%02dT%02d.%02d.%02d")
	% (1900 + currenttime->tm_year) % (currenttime->tm_mon + 1) % currenttime->tm_mday % currenttime->tm_hour % currenttime->tm_min % currenttime->tm_sec);
	//printf ("%s\n", timestamp.c_str()); // NOCOM

	std::string full_filename = get_base_dir() + g_fs->file_separator() + timestamp + "_ai_player_" + std::to_string(static_cast<int16_t>(pn)) + "." + AI_SUFFIX;

	printf(" %d: AI to be dumped to %s\n", pn, full_filename.c_str());

	Profile prof;

	Section& mn = prof.create_section("magic_numbers");
	assert(pd->magic_numbers_size == pd->magic_numbers.size());
	for (uint16_t i = 0; i < pd->magic_numbers_size; ++i) {
		mn.set_int(std::to_string(static_cast<int32_t>(i)).c_str(), pd->magic_numbers[i]);
	}

	Section& nv = prof.create_section("neuron_values");
	assert(pd->neuron_pool_size == pd->neuron_weights.size());
	for (uint16_t i = 0; i < pd->neuron_pool_size; ++i) {
		nv.set_int(std::to_string(static_cast<int32_t>(i)).c_str(), pd->neuron_weights[i]);
	}

	Section& nf = prof.create_section("neuron_functions");
	assert(pd->neuron_pool_size == pd->neuron_functs.size());
	for (uint16_t i = 0; i < pd->neuron_pool_size; ++i) {
		nf.set_int(std::to_string(static_cast<int32_t>(i)).c_str(), pd->neuron_functs[i]);
	}

	Section& fn = prof.create_section("fneurons");
	assert(pd->f_neuron_pool_size == pd->f_neurons.size());
	for (uint16_t i = 0; i < pd->f_neuron_pool_size; ++i) {
		fn.set_natural(std::to_string(static_cast<int64_t>(i)).c_str(), pd->f_neurons[i]);
	}
	prof.write(full_filename.c_str(), false, *g_fs);
}
}
