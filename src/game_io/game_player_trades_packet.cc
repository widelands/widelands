/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "game_io/game_player_trades_packet.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/market.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/**
 * Changelog:
 * 1: v1.3
 */
constexpr uint16_t kCurrentPacketVersion = 1;

void GamePlayerTradesPacket::read(FileSystem& fs, Game& game, MapObjectLoader* mol) {
	try {
		FileRead fr;
		if (!fr.try_open(fs, "binary/player_trades")) {
			// TODO(Nordfriese): Require this packet after v1.3
			log_warn("GamePlayerTradesPacket: binary file not present, assuming no trades");
			return;
		}

		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			for (size_t i = fr.unsigned_32(); i > 0; --i) {
				const TradeID id = fr.unsigned_32();
				game.next_trade_agreement_id_ = std::max(game.next_trade_agreement_id_, id + 1);
				assert(game.trade_agreements_.count(id) == 0);
				TradeInstance& trade = game.trade_agreements_[id];

				trade.state = static_cast<TradeInstance::State>(fr.unsigned_8());
				Serial serial = trade.state == TradeInstance::State::kProposed ? 0 : fr.unsigned_32();
				trade.receiver = serial == 0 ? nullptr : &mol->get<Market>(serial);

				serial = fr.unsigned_32();
				trade.initiator = serial == 0 ? nullptr : &mol->get<Market>(serial);

				trade.num_batches = fr.unsigned_32();
				trade.sending_player = fr.unsigned_8();
				trade.receiving_player = fr.unsigned_8();

				for (size_t j = fr.unsigned_32(); j > 0; --j) {
					uint32_t di = fr.unsigned_32();
					uint32_t amount = fr.unsigned_32();
					trade.items_to_send.emplace_back(di, amount);
				}

				for (size_t j = fr.unsigned_32(); j > 0; --j) {
					uint32_t di = fr.unsigned_32();
					uint32_t amount = fr.unsigned_32();
					trade.items_to_receive.emplace_back(di, amount);
				}
			}
		} else {
			throw UnhandledVersionError(
			   "GamePlayerEconomiesPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("economies: %s", e.what());
	}
}

/*
 * Write Function
 */
void GamePlayerTradesPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const mos) {
	FileWrite fw;
	fw.unsigned_16(kCurrentPacketVersion);

	fw.unsigned_32(game.trade_agreements_.size());
	for (const auto& pair : game.trade_agreements_) {
		fw.unsigned_32(pair.first);
		fw.unsigned_8(static_cast<uint8_t>(pair.second.state));
		if (pair.second.state != TradeInstance::State::kProposed) {
			fw.unsigned_32(mos->get_object_file_index_or_zero(pair.second.receiver.get(game)));
		}
		fw.unsigned_32(mos->get_object_file_index_or_zero(pair.second.initiator.get(game)));
		fw.unsigned_32(pair.second.num_batches);
		fw.unsigned_8(pair.second.sending_player);
		fw.unsigned_8(pair.second.receiving_player);
		fw.unsigned_32(pair.second.items_to_send.size());
		for (const WareAmount& amount : pair.second.items_to_send) {
			fw.unsigned_32(amount.first);
			fw.unsigned_32(amount.second);
		}
		fw.unsigned_32(pair.second.items_to_receive.size());
		for (const WareAmount& amount : pair.second.items_to_receive) {
			fw.unsigned_32(amount.first);
			fw.unsigned_32(amount.second);
		}
	}

	fw.write(fs, "binary/player_trades");
}
}  // namespace Widelands
