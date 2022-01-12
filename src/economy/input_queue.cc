/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#include "economy/input_queue.h"

#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/request.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

InputQueue::InputQueue(PlayerImmovable& init_owner,
                       DescriptionIndex const init_index,
                       uint8_t const init_max_size,
                       WareWorker const init_type)
   : owner_(init_owner),
     index_(init_index),
     max_size_(init_max_size),
     max_fill_(init_max_size),
     type_(init_type),
     consume_interval_(0),
     request_(nullptr),
     callback_fn_(nullptr),
     callback_data_(nullptr) {
}

void InputQueue::update() {

	assert(get_filled() <= max_size_);
	assert(max_fill_ <= max_size_);
	assert(index_ != INVALID_INDEX);

	if (get_filled() < max_fill_) {
		if (!request_) {
			request_.reset(new Request(owner_, index_, InputQueue::request_callback, type_));
			request_->set_exact_match(true);  // Required for worker queues, ignored for wares anyway
		}

		request_->set_count(max_fill_ - get_filled());
		request_->set_required_interval(consume_interval_);
	} else {
		request_.reset();
	}
}

void InputQueue::request_callback(Game& game,
                                  Request& r,
                                  DescriptionIndex const index,
                                  Worker* const worker,
                                  PlayerImmovable& target) {

	WareWorker type = wwWARE;
	if (worker != nullptr) {
		assert(index == worker->descr().worker_index());
		type = wwWORKER;
	}

	InputQueue& iq = dynamic_cast<Building&>(target).inputqueue(index, type, &r);

	iq.entered(index, worker);

	if (iq.callback_fn_) {
		(*iq.callback_fn_)(game, &iq, index, worker, iq.callback_data_);
	}
}

void InputQueue::set_callback(CallbackFn* const fn, void* const data) {
	callback_fn_ = fn;
	callback_data_ = data;
}

void InputQueue::set_max_size(const Quantity size) {
	Quantity old_size = max_size_;
	max_size_ = size;

	// make sure that max fill is reduced as well if the max size is decreased
	// because this is very likely what the user wanted to only consume so
	// and so many wares in the first place. If it is increased, keep the
	// max fill fill as it was
	set_max_fill(std::min(max_fill_, max_fill_ - (old_size - max_size_)));

	// No update() since set_max_fill calls it anyway
}

// I assume elsewhere that this function will call update()
// even if old and new size are identical
void InputQueue::set_max_fill(Quantity size) {
	if (size > max_size_) {
		size = max_size_;
	}

	max_fill_ = size;

	update();
}

void InputQueue::set_consume_interval(const Duration& time) {
	consume_interval_ = time;

	update();
}

uint32_t InputQueue::get_missing() const {
	const auto filled = get_filled();
	if (filled >= max_fill_ || request_ == nullptr || !request_->is_open()) {
		return 0;
	}
	return max_fill_ - filled - std::min(max_fill_, request_->get_num_transfers());
}

constexpr uint16_t kCurrentPacketVersion = 3;

void InputQueue::read(FileRead& fr, Game& game, MapObjectLoader& mol) {

	uint16_t const packet_version = fr.unsigned_16();
	try {
		if (packet_version == kCurrentPacketVersion) {
			if (fr.unsigned_8() == 0) {
				assert(type_ == wwWARE);
				index_ = game.descriptions().safe_ware_index(fr.c_string());
			} else {
				assert(type_ == wwWORKER);
				index_ = game.descriptions().safe_worker_index(fr.c_string());
			}
			max_size_ = fr.unsigned_32();
			max_fill_ = fr.signed_32();
			consume_interval_ = Duration(fr);
			if (fr.unsigned_8()) {
				request_.reset(new Request(owner_, 0, InputQueue::request_callback, type_));
				request_->read(fr, game, mol);
			} else {
				request_.reset();
			}

			read_child(fr, game, mol);
		} else {
			throw UnhandledVersionError("InputQueue", packet_version, kCurrentPacketVersion);
		}
		//  Now Economy stuff. We have to add our filled items to the economy.
		if (owner_.get_economy(type_)) {
			add_to_economy(*owner_.get_economy(type_));
		}
	} catch (const GameDataError& e) {
		throw GameDataError("inputqueue: %s", e.what());
	}
}

void InputQueue::write(FileWrite& fw, Game& game, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersion);
	assert(index_ != Widelands::INVALID_INDEX);

	//  Owner and callback is not saved, but this should be obvious on load.
	switch (type_) {
	case wwWARE:
		fw.unsigned_8(0);
		fw.c_string(owner().tribe().get_ware_descr(index_)->name().c_str());
		break;
	case wwWORKER:
		fw.unsigned_8(1);
		fw.c_string(owner().tribe().get_worker_descr(index_)->name().c_str());
		break;
	}
	fw.signed_32(max_size_);
	fw.signed_32(max_fill_);
	consume_interval_.save(fw);
	if (request_) {
		fw.unsigned_8(1);
		request_->write(fw, game, mos);
	} else {
		fw.unsigned_8(0);
	}

	write_child(fw, game, mos);
}
}  // namespace Widelands
