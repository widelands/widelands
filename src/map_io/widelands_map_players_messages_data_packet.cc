/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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

#include "map_io/widelands_map_players_messages_data_packet.h"

#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "profile/profile.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1
#define PLAYERDIRNAME_TEMPLATE "player/%u"
#define FILENAME_TEMPLATE PLAYERDIRNAME_TEMPLATE "/messages"
#define FILENAME_SIZE 19

void Map_Players_Messages_Data_Packet::Read
	(FileSystem & fs, Editor_Game_Base & egbase, bool, Map_Map_Object_Loader & mol)

{
	uint32_t      const gametime   = egbase.get_gametime ();
	const Map   &       map        = egbase.map          ();
	Extent        const extent     = map   .extent       ();
	Player_Number const nr_players = map   .get_nrplayers();
	iterate_players_existing(p, nr_players, egbase, player)
		try {
			char filename[FILENAME_SIZE];
			snprintf(filename, sizeof(filename), FILENAME_TEMPLATE, p);
			Profile prof;
			try {prof.read(filename, nullptr, fs);} catch (...) {continue;}
			prof.get_safe_section("global").get_positive
				("packet_version", CURRENT_PACKET_VERSION);
			MessageQueue & messages = player->messages();

			{
				MessageQueue::const_iterator const begin = messages.begin();
				if (begin != messages.end()) {
					log
						("ERROR: The message queue for player %1$u contains a message "
						 "before any messages have been loade into it. This is a bug "
						 "in the savegame loading code. It created a new message and "
						 "added it to the queue. This is only allowed during "
						 "simulation, not at load. The following messge will be "
						 "removed when the queue is reset:\n"
						 "\tsender  : %2$s\n"
						 "\ttitle   : %3$s\n"
						 "\tsent    : %4$u\n"
						 "\tduration: %5$u\n"
						 "\tposition: (%6$i, %7$i)\n"
						 "\tstatus  : %8$u\n"
						 "\tbody    : %9$s\n",
						 p,
						 begin->second->sender  ().c_str(),
						 begin->second->title   ().c_str(),
						 begin->second->sent    (),
						 begin->second->duration(),
						 begin->second->position().x, begin->second->position().y,
						 begin->second->status  (),
						 begin->second->body    ().c_str());
					messages.clear();
				}
			}

			uint32_t previous_message_sent = 0;
			while (Section * const s = prof.get_next_section())
				try {
					uint32_t const sent    = s->get_safe_int("sent");
					if (sent < previous_message_sent)
						throw game_data_error
							(_
							 	("messages are not ordered: sent at %1$u but previous "
							 	 "message sent at %2$u"),
							 sent, previous_message_sent);
					if (gametime < sent)
						throw game_data_error
							(_
							 	("message is sent in the future: sent at %1$u but "
							 	 "gametime is only %2$u"),
							 sent, gametime);
					uint32_t duration = Forever(); //  default duration
					if (Section::Value const * const dv = s->get_val("duration")) {
						duration = dv->get_positive();
						if (duration == Forever())
							throw game_data_error
								(_
								 	("the value %u is not allowed as duration; it is "
								 	 "a special value meaning forever, which is the "
								 	 "default; omit the duration key to make the "
								 	 "message exist forever"),
								 Forever());
						if (sent + duration < sent)
							throw game_data_error
								(_
								 	("duration %1$u is too large; causes numeric "
								 	 "overflow when added to sent time %2$u"),
								 duration, sent);
						if (sent + duration < gametime)
							throw game_data_error
								(_
								 	("message should have expired at %1$u; sent at %2$u "
								 	 "with duration %3$u but gametime is already %4$u"),
								 sent + duration, sent, duration, gametime);
					}
					Message::Status status = Message::Archived; //  default status
					if (char const * const status_string = s->get_string("status")) {
						try {
							if      (not strcmp(status_string, "new"))
								status = Message::New;
							else if (not strcmp(status_string, "read"))
								status = Message::Read;
							else
								throw game_data_error
									(_("expected %1$s but found \"%2$s\""),
									 "{new|read}", status_string);
						} catch (const _wexception & e) {
							throw game_data_error("status: %s", e.what());
						}
					}
					Serial serial = s->get_int("serial", 0);
					if (serial > 0) {
						assert(mol.is_object_known(serial));
						Map_Object & mo = mol.get<Map_Object>(serial);
						assert(mol.is_object_loaded(mo));
						serial = mo.serial();
					}

					messages.add_message
						(*new Message
						 	(s->get_string     ("sender", ""),
						 	 sent,
						 	 duration,
						 	 s->get_name       (),
						 	 s->get_safe_string("body"),
						 	 s->get_Coords     ("position", extent, Coords::Null()),
							 serial,
						 	 status));
					//  Expiration is scheduled for all messages (with finite
					//  duration) after the command queue has been loaded (in
					//  Game_Loader::load_game).
					previous_message_sent = sent;
				} catch (const _wexception & e) {
					throw game_data_error
						(_("\"%1$s\": %2$s"), s->get_name(), e.what());
				}
				prof.check_used();
		} catch (const _wexception & e) {
			throw game_data_error
				(_("messages for player %1$u: %2$s"), p, e.what());
		}
}

void Map_Players_Messages_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fs.EnsureDirectoryExists("player");
	Player_Number const nr_players = egbase.map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, egbase, player) {
		Profile prof;
		prof.create_section("global").set_int
			("packet_version", CURRENT_PACKET_VERSION);
		const MessageQueue & messages = player->messages();
		Map_Message_Saver & message_saver = mos.message_savers[p - 1];
		container_iterate_const(MessageQueue, messages, i) {
			message_saver.add         (i.current->first);
			const Message & message = *i.current->second;
			assert(message.sent() <= static_cast<uint32_t>(egbase.get_gametime()));
			assert
				(message.duration() == Forever() or
				 message.sent() < message.sent() + message.duration());
			if
				(message.duration() != Forever() and
				 message.sent() + message.duration()
				 <
				 static_cast<uint32_t>(egbase.get_gametime()))
				log
					("ERROR: Trying to save a message that should have expired:\n"
					 "\tsent = %1$u, duration = %2$u, expiry = %3$u, gametime = %4$u\n"
					 "\tsender = \"%5$s\"\n"
					 "\ttitle: %6$s\n"
					 "\tbody: %7$s\n"
					 "\tposition: (%8$i, %9$i)\n"
					 "\tstatus: %10$s\n",
					 message.sent(), message.duration(),
					 message.sent() + message.duration(), egbase.get_gametime(),
					 message.sender().c_str(), message.title().c_str(),
					 message.body().c_str(),
					 message.position().x, message.position().y,
					 message.status() == Message::New      ? "new"      :
					 message.status() == Message::Read     ? "read"     :
					 message.status() == Message::Archived ? "archived" : "ERROR");
			assert
				(message.duration() == Forever() or
				 static_cast<uint32_t>(egbase.get_gametime())
				 <=
				 message.sent() + message.duration());
			Section & s = prof.create_section_duplicate(message.title().c_str());
			if (message.sender().size())
				s.set_string("sender",    message.sender  ());
			s.set_int      ("sent",      message.sent    ());
			{
				Duration const duration = message.duration();
				if (duration != Forever()) //  The default duration. Do not write.
					s.set_int("duration",  duration);
			}
			s.set_string   ("body",      message.body    ());
			if (Coords const c =         message.position())
				s.set_Coords("position",  c);
			switch (message.status()) {
			case Message::New:
				s.set_string("status",    "new");
				break;
			case Message::Read:
				s.set_string("status",    "read");
				break;
			case Message::Archived: //  The default status. Do not write.
				break;
			default:
				assert(false);
			}
			if (message.serial()) {
				const Map_Object* mo = egbase.objects().get_object(message.serial());
				uint32_t fileindex = mos.get_object_file_index_or_zero(mo);
				s.set_int       ("serial",    fileindex);
			}
		}
		char filename[FILENAME_SIZE];
		snprintf(filename, sizeof(filename), PLAYERDIRNAME_TEMPLATE, p);
		fs.EnsureDirectoryExists(filename);
		snprintf(filename, sizeof(filename),      FILENAME_TEMPLATE, p);
		prof.write(filename, false, fs);
	}
}

}
