/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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

#include "map_io/map_players_messages_packet.h"

#include <memory>

#include "base/log.h"
#include "io/profile.h"
#include "logic/game_data_error.h"
#include "logic/message.h"
#include "logic/player.h"
#include "map_io/coords_profile.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint32_t kCurrentPacketVersion = 2;

constexpr const char* kPlayerDirnameTemplate = "player/%u";
constexpr const char* kFilenameTemplate = "player/%u/messages";

void MapPlayersMessagesPacket::read(FileSystem& fs,
                                    EditorGameBase& egbase,
                                    bool /* skip */,
                                    MapObjectLoader& mol)

{
	const Time& gametime = egbase.get_gametime();
	const Map& map = egbase.map();
	Extent const extent = map.extent();
	PlayerNumber const nr_players = map.get_nrplayers();
	iterate_players_existing(p, nr_players, egbase, player) try {
		Profile prof;
		try {
			const std::string profile_filename =
			   format(kFilenameTemplate, static_cast<unsigned int>(p));
			prof.read(profile_filename.c_str(), nullptr, fs);
		} catch (...) {
			continue;
		}
		uint32_t packet_version = prof.get_safe_section("global").get_positive("packet_version");
		if (1 <= packet_version && packet_version <= kCurrentPacketVersion) {
			MessageQueue* messages = player->get_messages();

			{
				const auto begin = messages->begin();
				if (begin != messages->end()) {
					log_err("ERROR: The message queue for player %u contains a message "
					        "before any messages have been loaded into it. This is a bug "
					        "in the savegame loading code. It created a new message and "
					        "added it to the queue. This is only allowed during "
					        "simulation, not at load. The following message will be "
					        "removed when the queue is reset:\n"
					        "\tstype   : %u\n"
					        "\ttitle   : %s\n"
					        "\tsent    : %u\n"
					        "\tposition: (%i, %i)\n"
					        "\tstatus  : %u\n"
					        "\tbody    : %s\n",
					        p, static_cast<int>(begin->second->type()), begin->second->title().c_str(),
					        begin->second->sent().get(), begin->second->position().x,
					        begin->second->position().y, static_cast<int>(begin->second->status()),
					        begin->second->body().c_str());
					messages->clear();
				}
			}

			Time previous_message_sent(0);
			while (Section* const s = prof.get_next_section()) {
				try {
					const Time sent(s->get_safe_int("sent"));
					if (sent < previous_message_sent) {
						throw GameDataError("messages are not ordered: sent at %u but previous "
						                    "message sent at %u",
						                    sent.get(), previous_message_sent.get());
					}
					if (gametime < sent) {
						throw GameDataError("message is sent in the future: sent at %u but "
						                    "gametime is only %u",
						                    sent.get(), gametime.get());
					}

					Message::Status status = Message::Status::kArchived;  //  default status
					if (char const* const status_string = s->get_string("status")) {
						try {
							if (!strcmp(status_string, "new")) {
								status = Message::Status::kNew;
							} else if (!strcmp(status_string, "read")) {
								status = Message::Status::kRead;
							} else {
								throw GameDataError(
								   "expected %s but found \"%s\"", "{new|read}", status_string);
							}
						} catch (const WException& e) {
							throw GameDataError("status: %s", e.what());
						}
					}
					Serial serial = s->get_int("serial", 0);
					if (serial > 0) {
						assert(mol.is_object_known(serial));
						MapObject& mo = mol.get<MapObject>(serial);
						assert(mol.is_object_loaded(mo));
						serial = mo.serial();
					}
					// Compatibility code needed for map loading.
					if (packet_version == 1) {
						const std::string name = s->get_name();
						messages->add_message(std::unique_ptr<Message>(new Message(
						   static_cast<Message::Type>(s->get_natural("type")), sent, name,
						   "images/wui/fieldaction/menu_build_flag.png", name, s->get_safe_string("body"),
						   get_coords("position", extent, Coords::null(), s), serial, "", status)));
					} else {

						messages->add_message(std::unique_ptr<Message>(new Message(
						   static_cast<Message::Type>(s->get_natural("type")), sent, s->get_name(),
						   s->get_safe_string("icon"), s->get_safe_string("heading"),
						   s->get_safe_string("body"), get_coords("position", extent, Coords::null(), s),
						   serial, std::string(s->get_string("subtype", "")), status)));
					}
					previous_message_sent = sent;
				} catch (const WException& e) {
					throw GameDataError("\"%s\": %s", s->get_name(), e.what());
				}
			}
			prof.check_used();
		} else {
			throw UnhandledVersionError(
			   "MapPlayersMessagesPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("messages for player %u: %s", p, e.what());
	}
}

void MapPlayersMessagesPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos) {
	fs.ensure_directory_exists("player");
	PlayerNumber const nr_players = egbase.map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, egbase, player) {
		Profile prof;
		prof.create_section("global").set_int("packet_version", kCurrentPacketVersion);
		const MessageQueue& messages = player->messages();
		MapMessageSaver& message_saver = mos.message_savers[p - 1];
		for (const auto& temp_message : messages) {
			message_saver.add(temp_message.first);
			const Message& message = *temp_message.second;
			assert(message.sent() <= egbase.get_gametime());

			Section& s = prof.create_section_duplicate(message.title().c_str());
			s.set_int("type", static_cast<int32_t>(message.type()));
			s.set_string("heading", message.heading());
			s.set_string("icon", message.icon_filename());
			s.set_int("sent", message.sent().get());
			s.set_string("body", message.body());
			if (Coords const c = message.position()) {
				set_coords("position", c, &s);
			}
			switch (message.status()) {
			case Message::Status::kNew:
				s.set_string("status", "new");
				break;
			case Message::Status::kRead:
				s.set_string("status", "read");
				break;
			case Message::Status::kArchived:  //  The default status. Do not write.
				break;
			}
			if (message.serial()) {
				const MapObject* mo = egbase.objects().get_object(message.serial());
				uint32_t fileindex = mos.get_object_file_index_or_zero(mo);
				s.set_int("serial", fileindex);
			}
			s.set_string("subtype", message.sub_type().c_str());
		}
		fs.ensure_directory_exists(format(kPlayerDirnameTemplate, static_cast<unsigned int>(p)));

		const std::string profile_filename = format(kFilenameTemplate, static_cast<unsigned int>(p));
		prof.write(profile_filename.c_str(), false, fs);
	}
}
}  // namespace Widelands
