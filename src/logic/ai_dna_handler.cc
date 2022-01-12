/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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

#include "base/log.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"

namespace Widelands {

AiDnaHandler::AiDnaHandler() {
	g_fs->ensure_directory_exists(kAiDir);
}

// This reads the AI file for a particular slot (position) and populates numbers into passed
// vectors.
// Slots are numbers 1 - 4 as we have 4 possible input files with an AI's DNA.
void AiDnaHandler::fetch_dna(std::vector<int16_t>& military_numbers,
                             std::vector<int8_t>& input_weights,
                             std::vector<int8_t>& input_func,
                             std::vector<uint32_t>& f_neurons,
                             uint8_t slot) {

	// AI files are in range 1-4
	assert(slot > 0 && slot < 5);

	const std::string full_filename = kAiDir + FileSystem::file_separator() +
	                                  std::string("ai_input_") +
	                                  std::to_string(static_cast<int>(slot)) + kAiExtension;

	Profile prof;
	prof.read(full_filename.c_str(), nullptr, *g_fs);
	Section& mn = prof.get_safe_section("magic_numbers");
	for (size_t i = 0; i < military_numbers.size(); i++) {
		int32_t value = mn.get_int(std::to_string(static_cast<int32_t>(i)).c_str());
		if (value < -100 || value > 100) {
			throw wexception(
			   "Out of range AI data in magic_numbers section: %d\n", static_cast<int>(i));
		}
		military_numbers[i] = static_cast<int16_t>(value);
	}

	Section& nv = prof.get_safe_section("neuron_values");
	for (size_t i = 0; i < input_weights.size(); i++) {
		int32_t value = nv.get_int(std::to_string(static_cast<int32_t>(i)).c_str());
		if (value < -100 || value > 100) {
			throw wexception(
			   "Out of range AI data in neuron_values section: %d\n", static_cast<int>(i));
		}
		input_weights[i] = static_cast<int8_t>(value);
	}

	Section& nf = prof.get_safe_section("neuron_functions");
	for (size_t i = 0; i < input_func.size(); i++) {
		int32_t value = nf.get_int(std::to_string(static_cast<int32_t>(i)).c_str());
		if (value < 0 || value > 3) {
			throw wexception(
			   "Out of range  AI data in neuron_functions section: %d\n", static_cast<int>(i));
		}
		input_func[i] = static_cast<int8_t>(value);
	}

	Section& fn = prof.get_safe_section("fneurons");
	for (size_t i = 0; i < f_neurons.size(); i++) {
		f_neurons[i] = fn.get_natural(std::to_string(static_cast<int32_t>(i)).c_str());
	}
}

// This generates a new file with AI data in '.widelands/ai'
void AiDnaHandler::dump_output(Widelands::Player::AiPersistentState* pd, uint8_t pn) {

	const std::string full_filename = kAiDir + FileSystem::file_separator() +
	                                  std::string(timestring()) + std::string("_ai_player_") +
	                                  std::to_string(static_cast<int>(pn)) + kAiExtension;

	verb_log_dbg("AIPARSE %d: AI to be dumped to %s\n", pn, full_filename.c_str());

	Profile prof;

	Section& mn = prof.create_section("magic_numbers");
	assert(pd->magic_numbers.size() == Widelands::Player::AiPersistentState::kMagicNumbersSize);
	for (size_t i = 0; i < pd->magic_numbers.size(); ++i) {
		mn.set_int(std::to_string(static_cast<int32_t>(i)).c_str(), pd->magic_numbers.at(i));
	}

	Section& nv = prof.create_section("neuron_values");
	assert(pd->neuron_weights.size() == Widelands::Player::AiPersistentState::kNeuronPoolSize);
	for (size_t i = 0; i < pd->neuron_weights.size(); ++i) {
		nv.set_int(std::to_string(static_cast<int32_t>(i)).c_str(), pd->neuron_weights.at(i));
	}

	Section& nf = prof.create_section("neuron_functions");
	assert(pd->neuron_functs.size() == Widelands::Player::AiPersistentState::kNeuronPoolSize);
	for (size_t i = 0; i < pd->neuron_functs.size(); ++i) {
		nf.set_int(std::to_string(static_cast<int32_t>(i)).c_str(), pd->neuron_functs.at(i));
	}

	Section& fn = prof.create_section("fneurons");
	assert(pd->f_neurons.size() == Widelands::Player::AiPersistentState::kFNeuronPoolSize);
	for (size_t i = 0; i < pd->f_neurons.size(); ++i) {
		fn.set_natural(std::to_string(static_cast<int64_t>(i)).c_str(), pd->f_neurons.at(i));
	}

	std::string comment = "See wiki for more info: https://www.widelands.org/wiki/Ai%20Training/";

	prof.write(full_filename.c_str(), false, *g_fs, comment.c_str());
}
}  // namespace Widelands
