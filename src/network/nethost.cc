/*
 * Copyright (C) 2008-2013 by the Widelands Development Team
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

#include "network/nethost.h"

#include <sstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <libintl.h>
#ifndef _WIN32
#include <unistd.h> // for usleep
#endif

#include "build_info.h"
#include "chat.h"
#include "computer_player.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_data_packet.h"
#include "helper.h"
#include "i18n.h"
#include "io/dedicated_log.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/playersmanager.h"
#include "logic/tribe.h"
#include "map_io/widelands_map_loader.h"
#include "md5.h"
#include "network/internet_gaming.h"
#include "network/network_gaming_messages.h"
#include "network/network_lan_promotion.h"
#include "network/network_player_settings_backend.h"
#include "network/network_protocol.h"
#include "network/network_system.h"
#include "profile/profile.h"
#include "scripting/scripting.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/launchMPG.h"
#include "wexception.h"
#include "wlapplication.h"
#include "wui/game_tips.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

using boost::format;



struct HostGameSettingsProvider : public GameSettingsProvider {
	HostGameSettingsProvider(NetHost * const _h) : h(_h), m_lua(nullptr), m_cur_wincondition(0) {}
	~HostGameSettingsProvider() {
		delete m_lua;
		m_lua = nullptr;
	}

	virtual void setScenario(bool is_scenario) override {h->setScenario(is_scenario);}

	virtual const GameSettings & settings() override {return h->settings();}

	virtual bool canChangeMap() override {return true;}
	virtual bool canChangePlayerState(uint8_t const number) override {
		if (settings().savegame)
			return settings().players.at(number).state != PlayerSettings::stateClosed;
		else if (settings().scenario)
			return
				((settings().players.at(number).state == PlayerSettings::stateOpen
				  ||
				  settings().players.at(number).state == PlayerSettings::stateHuman)
				 &&
				 settings().players.at(number).closeable)
				||
				settings().players.at(number).state == PlayerSettings::stateClosed;
		return true;
	}
	virtual bool canChangePlayerTribe(uint8_t const number) override {
		return canChangePlayerTeam(number);
	}
	virtual bool canChangePlayerInit(uint8_t const number) override {
		if (settings().scenario || settings().savegame)
			return false;
		return number < settings().players.size();
	}
	virtual bool canChangePlayerTeam(uint8_t number) override {
		if (settings().scenario || settings().savegame)
			return false;
		if (number >= settings().players.size())
			return false;
		if (number == settings().playernum)
			return true;
		return
			settings().players.at(number).state == PlayerSettings::stateComputer;
	}

	virtual bool canLaunch() override {return h->canLaunch();}

	virtual void setMap
		(const std::string &       mapname,
		 const std::string &       mapfilename,
		 uint32_t            const maxplayers,
		 bool                const savegame = false) override
	{
		h->setMap(mapname, mapfilename, maxplayers, savegame);
	}
	virtual void setPlayerState
		(uint8_t const number, PlayerSettings::State const state) override
	{
		if (number >= settings().players.size())
			return;

		h->setPlayerState(number, state);
	}
	virtual void nextPlayerState(uint8_t const number) override {
		if (number > settings().players.size())
			return;

		PlayerSettings::State newstate = PlayerSettings::stateClosed;
		switch (h->settings().players.at(number).state) {
		case PlayerSettings::stateClosed:
			// In savegames : closed players can not be changed.
			assert(!h->settings().savegame);
			newstate = PlayerSettings::stateOpen;
			break;
		case PlayerSettings::stateOpen:
		case PlayerSettings::stateHuman:
			if (h->settings().scenario) {
				assert(h->settings().players.at(number).closeable);
				newstate = PlayerSettings::stateClosed;
				break;
			} // else fall through
			/* no break */
		case PlayerSettings::stateComputer:
			{
				const Computer_Player::ImplementationVector & impls =
					Computer_Player::getImplementations();
				Computer_Player::ImplementationVector::const_iterator it =
					impls.begin();
				if (h->settings().players.at(number).ai.empty()) {
					setPlayerAI(number, (*it)->name);
					newstate = PlayerSettings::stateComputer;
					break;
				}
				do {
					++it;
					if ((*(it - 1))->name == h->settings().players.at(number).ai)
						break;
				} while (it != impls.end());
				if (settings().players.at(number).random_ai) {
					setPlayerAI(number, std::string());
					setPlayerName(number, std::string());
					// Do not share a player in savegames or scenarios
					if (h->settings().scenario || h->settings().savegame)
						newstate = PlayerSettings::stateOpen;
					else {
						uint8_t shared = 0;
						for (; shared < settings().players.size(); ++shared) {
							if
								(settings().players.at(shared).state != PlayerSettings::stateClosed
								 &&
								 settings().players.at(shared).state != PlayerSettings::stateShared)
								break;
						}
						if (shared < settings().players.size()) {
							newstate = PlayerSettings::stateShared;
							setPlayerShared(number, shared + 1);
						} else
							newstate = PlayerSettings::stateClosed;
					}
				} else if (it == impls.end()) {
					do {
						uint8_t random = (std::rand() % impls.size()); // Choose a random AI
						it = impls.begin() + random;
					} while ((*it)->name == "None");
					setPlayerAI(number, (*it)->name, true);
					newstate = PlayerSettings::stateComputer;
					break;
				} else {
					setPlayerAI(number, (*it)->name);
					newstate = PlayerSettings::stateComputer;
				}
				break;
			}
		case PlayerSettings::stateShared:
			{
				// Do not close a player in savegames or scenarios
				if (h->settings().scenario || h->settings().savegame)
					newstate = PlayerSettings::stateOpen;
				else
					newstate = PlayerSettings::stateClosed;
				break;
			}
		default:;
		}

		h->setPlayerState(number, newstate, true);
	}

	virtual void setPlayerTribe
		(uint8_t const number, const std::string & tribe, bool const random_tribe) override
	{
		if (number >= h->settings().players.size())
			return;

		if
			(number == settings().playernum
			 ||
			 settings().players.at(number).state == PlayerSettings::stateComputer
			 ||
			 settings().players.at(number).state == PlayerSettings::stateShared
			 ||
			 settings().players.at(number).state == PlayerSettings::stateOpen // For savegame loading
			)
			h->setPlayerTribe(number, tribe, random_tribe);
	}

	virtual void setPlayerTeam(uint8_t number, Widelands::TeamNumber team) override
	{
		if (number >= h->settings().players.size())
			return;

		if
			(number == settings().playernum ||
			 settings().players.at(number).state == PlayerSettings::stateComputer)
			h->setPlayerTeam(number, team);
	}

	virtual void setPlayerCloseable(uint8_t number, bool closeable) override {
		if (number >= h->settings().players.size())
			return;
		h->setPlayerCloseable(number, closeable);
	}

	virtual void setPlayerShared(uint8_t number, uint8_t shared) override {
		if (number >= h->settings().players.size())
			return;
		h->setPlayerShared(number, shared);
	}

	virtual void setPlayerInit(uint8_t const number, uint8_t const index) override {
		if (number >= h->settings().players.size())
			return;

		h->setPlayerInit(number, index);
	}

	virtual void setPlayerAI(uint8_t number, const std::string & name, bool const random_ai = false) override {
		h->setPlayerAI(number, name, random_ai);
	}

	virtual void setPlayerName(uint8_t const number, const std::string & name) override {
		if (number >= h->settings().players.size())
			return;
		h->setPlayerName(number, name);
	}

	virtual void setPlayer(uint8_t const number, PlayerSettings const ps) override {
		if (number >= h->settings().players.size())
			return;
		h->setPlayer(number, ps);
	}

	virtual void setPlayerNumber(uint8_t const number) override {
		if
			(number == UserSettings::none() or
			 number < h->settings().players.size())
			h->setPlayerNumber(number);
	}

	virtual std::string getWinConditionScript() override {
		return h->settings().win_condition_script;
	}

	virtual void setWinConditionScript(std::string wc) override {
		h->setWinConditionScript(wc);
	}

	virtual void nextWinCondition() override {
		if (m_win_condition_scripts.size() < 1) {
			if (!m_lua)
				m_lua = new LuaInterface();
			std::set<std::string> win_conditions =
			   filter(g_fs->ListDirectory("scripting/win_conditions"),
			          [](const std::string& fn) {return boost::ends_with(fn, ".lua");});
			m_win_condition_scripts.insert(
			   m_win_condition_scripts.end(), win_conditions.begin(), win_conditions.end());
			m_cur_wincondition = -1;
		}

		if (canChangeMap()) {
			m_cur_wincondition++;
			m_cur_wincondition %= m_win_condition_scripts.size();
			setWinConditionScript(m_win_condition_scripts[m_cur_wincondition]);
		}
	}

private:
	NetHost                * h;
	LuaInterface           * m_lua;
	int16_t                  m_cur_wincondition;
	std::vector<std::string> m_win_condition_scripts;
};

struct HostChatProvider : public ChatProvider {
	HostChatProvider(NetHost * const _h) : h(_h), kickClient(0) {}

	void send(const std::string & msg) override {
		ChatMessage c;
		c.time = time(nullptr);
		c.playern = h->getLocalPlayerposition();
		c.sender = h->getLocalPlayername();
		c.msg = msg;
		if (c.msg.size() && *c.msg.begin() == '@') {
			// Personal message
			std::string::size_type const space = c.msg.find(' ');
			if (space >= c.msg.size() - 1)
				return;
			c.recipient = c.msg.substr(1, space - 1);
			c.msg = c.msg.substr(space + 1);
		}

		/* Handle hostcommands like:
		 * /help                : Shows all available commands
		 * /announce <msg>      : Send a chatmessage as announcement (system chat)
		 * /warn <name> <reason>: Warn the user <name> because of <reason>
		 * /kick <name> <reason>: Kick the user <name> because of <reason>
		 * /forcePause          : Force the game to pause.
		 * /endForcedPause      : Puts game back to normal speed.
		 */
		else if (c.msg.size() > 1 && *c.msg.begin() == '/') {

			// Split up in "cmd" "arg1" "arg2"
			std::string cmd, arg1, arg2;
			std::string temp = c.msg.substr(1); // cut off '/'
			h->splitCommandArray(temp, cmd, arg1, arg2);
			dedicatedlog("%s + \"%s\" + \"%s\"\n", cmd.c_str(), arg1.c_str(), arg2.c_str());

			// let "/me" pass - handled by chat
			if (cmd == "me") {
				h->send(c);
				return;
			}

			// Everything handled from now on will be system stuff - and so will be
			// messages send because of that commands
			c.playern = -2;
			c.sender = "";

			// Help
			if (cmd == "help") {
				c.msg =
					(boost::format("<br>%s<br>%s<br>%s<br>%s<br>%s<br>%s<br>%s")
						% _("Available host commands are:")
						/** TRANSLATORS: Available host command */
						% _("/help  -  Shows this help")
						/** TRANSLATORS: Available host command */
						% _("/announce <msg>  -  Send a chatmessage as announcement (system chat)")
						/** TRANSLATORS: Available host command */
						% _("/warn <name> <reason>  -  Warn the user <name> because of <reason>")
						/** TRANSLATORS: Available host command */
						% _("/kick <name> <reason>  -  Kick the user <name> because of <reason>")
						/** TRANSLATORS: Available host command */
						% _("/forcePause            -  Force the game to pause.")
						/** TRANSLATORS: Available host command */
						% _("/endForcedPause        -  Return game to normal speed.")
					).str();
				if (!h->isDedicated())
					c.recipient = h->getLocalPlayername();
			}

			// Announce
			else if (cmd == "announce") {
				if (arg1.empty()) {
					c.msg = _("Wrong use, should be: /announce <message>");
					if (!h->isDedicated())
						c.recipient = h->getLocalPlayername();
				} else {
					if (arg2.size())
						arg1 += " " + arg2;
					c.msg = "HOST ANNOUNCEMENT: " + arg1;
				}
			}

			// Warn user
			else if (cmd == "warn") {
				if (arg1.empty() || arg2.empty()) {
					c.msg = _("Wrong use, should be: /warn <name> <reason>");
					if (!h->isDedicated())
						c.recipient = h->getLocalPlayername();
				} else {
					int32_t num = h->checkClient(arg1);
					if (num == -2) {
						if (!h->isDedicated()) {
							c.recipient = h->getLocalPlayername();
							c.msg = _("Why would you warn yourself?");
						} else
							c.msg = _("Why would you want to warn the dedicated server?");
					} else if (num == -1) {
						if (!h->isDedicated())
							c.recipient = h->getLocalPlayername();
						c.msg = (format(_("The client %s could not be found.")) % arg1).str();
					} else {
						c.msg  = (format("HOST WARNING FOR %s: ") % arg1).str();
						c.msg += arg2;
					}
				}
			}

			// Kick
			else if (cmd == "kick") {
				if (arg1.empty()) {
					c.msg = _("Wrong use, should be: /kick <name> <reason>");
				} else {
					kickUser = arg1;
					if (arg2.size())
						kickReason = arg2;
					else
						kickReason = "No reason given!";
					// Check if client exists
					int32_t num = h->checkClient(kickUser);
					if (num == -2)
						if (!h->isDedicated()) {
							c.msg = _("You can not kick yourself!");
						} else
							c.msg = _("You can not kick the dedicated server");
					else if (num == -1)
						c.msg = (format(_("The client %s could not be found.")) % arg1).str();
					else {
						kickClient = num;
						c.msg  = (format(_("Are you sure you want to kick %s?")) % arg1).str() + "<br>";
						c.msg += (format(_("The stated reason was: %s")) % kickReason).str() + "<br>";
						c.msg += (format(_("If yes, type: /ack_kick %s")) % arg1).str();
					}
				}
				if (!h->isDedicated())
					c.recipient = h->getLocalPlayername();
			}

			// Acknowledge kick
			else if (cmd == "ack_kick") {
				if (arg1.empty())
					c.msg = _("kick acknowledgement cancelled: No name given!");
				else if (arg2.size())
					c.msg = _("Wrong use, should be: /ack_kick <name>");
				else {
					if (arg1 == kickUser) {
						h->kickUser(kickClient, kickReason);
						return;
					} else
						c.msg = _("kick acknowledgement cancelled: Wrong name given!");
				}
				kickUser   = "";
				kickReason = "";
				if (!h->isDedicated())
					c.recipient = h->getLocalPlayername();
			}

			// Force Pause
			else if (cmd == "forcePause") {
				if (h->forcedPause()) {
					c.msg = _("Pause was already forced - game should be paused.");
					if (!h->isDedicated())
						c.recipient = h->getLocalPlayername();
				} else {
					c.msg = "HOST FORCED THE GAME TO PAUSE!";
					h->forcePause();
				}
			}

			// End Forced Pause
			else if (cmd == "endForcedPause") {
				if (!h->forcedPause()) {
					c.msg = _("There is no forced pause - nothing to end.");
					if (!h->isDedicated())
						c.recipient = h->getLocalPlayername();
				} else {
					c.msg = "HOST ENDED THE FORCED GAME PAUSE!";
					h->endForcedPause();
				}
			}

			// Default
			else {
				c.msg = _("Invalid command! Type /help for a list of commands.");
				if (!h->isDedicated())
					c.recipient = h->getLocalPlayername();
			}
		}
		h->send(c);
	}

	const std::vector<ChatMessage> & getMessages() const override {
		return messages;
	}

	void receive(const ChatMessage & msg) {
		messages.push_back(msg);
		ChatProvider::send(msg);
	}

private:
	NetHost                * h;
	std::vector<ChatMessage> messages;
	std::string              kickUser;
	uint32_t                 kickClient;
	std::string              kickReason;
};

struct Client {
	TCPsocket sock;
	Deserializer deserializer;
	uint8_t playernum;
	int16_t usernum;
	std::string build_id;
	md5_checksum syncreport;
	bool syncreport_arrived;
	int32_t time; // last time report
	uint32_t desiredspeed;
	bool dedicated_access;
	time_t hung_since;
	/// The delta time where the last information about the hung client was sent to the other clients relative
	/// to when the last answer of the client was received.
	time_t  lastdelta;
};

struct NetHostImpl {
	GameSettings settings;
	std::string localplayername;
	uint32_t localdesiredspeed;
	HostChatProvider chat;
	HostGameSettingsProvider hp;
	NetworkPlayerSettingsBackend npsb;

	LAN_Game_Promoter * promoter;
	TCPsocket svsock;
	SDLNet_SocketSet sockset;

	/// List of connected clients. Note that clients are not in the same
	/// order as players. In fact, a client must not be assigned to a player.
	std::vector<Client> clients;

	/// Set to true, once the dedicated server should start
	bool dedicated_start;

	/// The game itself; only non-null while game is running
	Widelands::Game * game;

	/// If we were to send out a plain networktime packet, this would be the
	/// time. However, we have not yet committed to this networktime.
	int32_t pseudo_networktime;
	int32_t last_heartbeat;

	/// The networktime we committed to by sending it across the network.
	int32_t committed_networktime;

	/// This is the time for local simulation
	NetworkTime time;

	/// Whether we're waiting for all clients to report back.
	bool    waiting;
	int32_t lastframe;

	/**
	 * The speed, in milliseconds per second, that is effective as long
	 * as we're not \ref waiting.
	 */
	uint32_t networkspeed;
	time_t   lastpauseping;

	/// All currently running computer players, *NOT* in one-one correspondence
	/// with \ref Player objects
	std::vector<Computer_Player *> computerplayers;

	/// \c true if a syncreport is currently in flight
	bool syncreport_pending;
	int32_t syncreport_time;
	md5_checksum syncreport;
	bool syncreport_arrived;

	NetHostImpl(NetHost * const h) :
		localdesiredspeed(0),
		chat(h),
		hp(h),
		npsb(&hp),
		promoter(nullptr),
		svsock(nullptr),
		sockset(nullptr),
		game(nullptr),
		pseudo_networktime(0),
		last_heartbeat(0),
		committed_networktime(0),
		waiting(false),
		lastframe(0),
		networkspeed(0),
		lastpauseping(0),
		syncreport_pending(false),
		syncreport_time(0),
		syncreport_arrived(false)
	{
		dedicated_start = false;
	}
};

NetHost::NetHost (const std::string & playername, bool internet)
	:
	d(new NetHostImpl(this)),
	m_internet(internet),
	m_is_dedicated(false),
	m_password(""),
	m_dedicated_motd(""),
	m_forced_pause(false)
{
	dedicatedlog("[Host]: starting up.\n");

	if (internet) {
		InternetGaming::ref().open_game();
	}

	d->localplayername = playername;

	// create a listening socket
	IPaddress myaddr;
	SDLNet_ResolveHost (&myaddr, nullptr, WIDELANDS_PORT);
	d->svsock = SDLNet_TCP_Open(&myaddr);

	d->sockset = SDLNet_AllocSocketSet(16);
	d->promoter = new LAN_Game_Promoter();
	d->game = nullptr;
	d->pseudo_networktime = 0;
	d->waiting = true;
	d->networkspeed = 1000;
	d->localdesiredspeed = 1000;
	d->syncreport_pending = false;
	d->syncreport_time = 0;

	d->settings.tribes = Widelands::Tribe_Descr::get_all_tribe_infos();
	setMultiplayerGameSettings();
	d->settings.playernum = UserSettings::none();
	d->settings.usernum = 0;
	UserSettings hostuser;
	hostuser.name = playername;
	hostuser.position = UserSettings::none();
	hostuser.ready = true;
	d->settings.users.push_back(hostuser);
	file = nullptr; //  Initialize as 0 pointer - unfortunately needed in struct.
}

NetHost::~NetHost ()
{
	clearComputerPlayers();

	while (d->clients.size() > 0) {
		disconnectClient(0, "SERVER_LEFT");
		reaper();
	}

	SDLNet_FreeSocketSet (d->sockset);

	// close all open sockets
	if (d->svsock != nullptr)
		SDLNet_TCP_Close (d->svsock);

	delete d->promoter;
	delete d;
	delete file;
}

const std::string & NetHost::getLocalPlayername() const
{
	return d->localplayername;
}

int16_t NetHost::getLocalPlayerposition()
{
	return d->settings.users.at(0).position;
}

void NetHost::clearComputerPlayers()
{
	for (uint32_t i = 0; i < d->computerplayers.size(); ++i)
		delete d->computerplayers.at(i);
	d->computerplayers.clear();
}

void NetHost::initComputerPlayer(Widelands::Player_Number p)
{
	d->computerplayers.push_back
		(Computer_Player::getImplementation(d->game->get_player(p)->getAI())->instantiate(*d->game, p));
}

void NetHost::initComputerPlayers()
{
	const Widelands::Player_Number nr_players = d->game->map().get_nrplayers();
	iterate_players_existing_novar(p, nr_players, *d->game) {
		if (p == d->settings.playernum + 1)
			continue;

		uint32_t client;
		for (client = 0; client < d->clients.size(); ++client)
			if (d->clients.at(client).playernum + 1 == p)
				break;

		if (client >= d->clients.size())
			initComputerPlayer(p);
	}
}

void NetHost::run(bool const autorun)
{
	m_is_dedicated = autorun;
	// Fill the list of possible system messages
	NetworkGamingMessages::fill_map();
	if (m_is_dedicated) {
		// Initializing
		d->hp.nextWinCondition();
		// May be the server is password protected?
		Section & s = g_options.pull_section("global");
		m_password  = s.get_string("dedicated_password", "");
		// And we read the message of the day
		m_dedicated_motd =
			s.get_string
				("dedicated_motd",
				 (format
					(_("This is a dedicated server. Send \"@%s help\" to get a full list of available commands."))
					% d->localplayername)
				.str().c_str());

		// Maybe this is the first run, so we try to setup the DedicatedLog
		// empty strings are treated as "do not write this type of log"
		DedicatedLog * dl = DedicatedLog::get();
		bool needip = false;
		if (!dl->set_log_file_path (s.get_string("dedicated_log_file_path",  "")))
			dedicatedlog("Warning: Could not set dedicated log file path");
		if (!dl->set_chat_file_path(s.get_string("dedicated_chat_file_path", "")))
			dedicatedlog("Warning: Could not set dedicated chat file path");
		if (!dl->write_info_active()) {
			if (!dl->set_info_file_path(s.get_string("dedicated_info_file_path", "")))
				dedicatedlog("Warning: Could not set dedicated info file path");
			else {
				needip = true;
				dl->set_server_data(InternetGaming::ref().get_local_servername(), m_dedicated_motd);
			}
		}

		dl->chatAddSpacer();
		// Setup by the users
		log ("[Dedicated] Entering set up mode, waiting for user interaction!\n");

		while (not d->dedicated_start) {
			handle_network();
			// TODO this should be improved.
#ifndef _WIN32
			if (d->clients.empty()) {
				if (usleep(100000)) // Sleep for 0.1 seconds - there is not anybody connected anyways.
					return;
				if (needip && (InternetGaming::ref().ip().size() < 1)) {
					dl->set_server_ip(InternetGaming::ref().ip());
					needip = false;
				}
			} else {
				if (usleep(200) == -1)
					return;
			}
#else
			if (d->clients.empty())
				Sleep(100);
			else
				Sleep(1);
#endif
		}
		d->dedicated_start = false;
	} else {
		Fullscreen_Menu_LaunchMPG lm(&d->hp, this);
		lm.setChatProvider(d->chat);
		const int32_t code = lm.run();
		if (code <= 0) {
			// if this is an internet game, tell the metaserver that client is back in the lobby.
			if (m_internet)
				InternetGaming::ref().set_game_done();
			return;
		}
	}

	// if this is an internet game, tell the metaserver that the game started
	if (m_internet)
		InternetGaming::ref().set_game_playing();

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients.at(i).playernum == UserSettings::notConnected())
			disconnectClient(i, "GAME_STARTED_AT_CONNECT");
	}

	SendPacket s;
	s.Unsigned8(NETCMD_LAUNCH);
	broadcast(s);

	Widelands::Game game;
#ifndef NDEBUG
	game.set_write_syncstream(true);
#endif

	try {
		// NOTE  loaderUI will stay uninitialized, if this is run as dedicated, so all called functions need
		// NOTE  to check whether the pointer is valid.
		std::unique_ptr<UI::ProgressWindow> loaderUI;
		GameTips * tips = nullptr;
		if (m_is_dedicated) {
			log ("[Dedicated] Starting the game...\n");
			d->game = &game;
			game.set_game_controller(this);

			if (d->settings.savegame) {
				// Read and broadcast original win condition
				Widelands::Game_Loader gl(d->settings.mapfilename, game);
				Widelands::Game_Preload_Data_Packet gpdp;
				gl.preload_game(gpdp);

				setWinConditionScript(gpdp.get_win_condition());
			}
		} else {
			loaderUI.reset(new UI::ProgressWindow ("pics/progress.png"));
			std::vector<std::string> tipstext;
			tipstext.push_back("general_game");
			tipstext.push_back("multiplayer");
			try {
				tipstext.push_back(d->hp.getPlayersTribe());
			} catch (GameSettingsProvider::No_Tribe) {}
			tips = new GameTips(*loaderUI, tipstext);

			loaderUI->step(_("Preparing game"));

			d->game = &game;
			game.set_game_controller(this);
			Interactive_GameBase * igb;
			uint8_t pn = d->settings.playernum + 1;

			if (d->settings.savegame) {
				// Read and broadcast original win condition
				Widelands::Game_Loader gl(d->settings.mapfilename, game);
				Widelands::Game_Preload_Data_Packet gpdp;
				gl.preload_game(gpdp);

				setWinConditionScript(gpdp.get_win_condition());
			}

			if ((pn > 0) && (pn <= UserSettings::highestPlayernum())) {
				igb =
					new Interactive_Player
						(game, g_options.pull_section("global"),
						pn, d->settings.scenario, true);
			} else
				igb =
					new Interactive_Spectator
						(game, g_options.pull_section("global"), true);
			igb->set_chat_provider(d->chat);
			game.set_ibase(igb);
		}

		if (!d->settings.savegame) // new game
			game.init_newgame (loaderUI.get(), d->settings);
		else                      // savegame
			game.init_savegame(loaderUI.get(), d->settings);
		d->pseudo_networktime = game.get_gametime();
		d->time.reset(d->pseudo_networktime);
		d->lastframe = WLApplication::get()->get_time();
		d->last_heartbeat = d->lastframe;

		d->committed_networktime = d->pseudo_networktime;

		for (uint32_t i = 0; i < d->clients.size(); ++i)
			d->clients.at(i).time = d->committed_networktime - 1;

		// The call to checkHungClients ensures that the game leaves the
		// wait mode when there are no clients
		checkHungClients();
		initComputerPlayers();
		if (m_is_dedicated) {
			// Statistics: new game started
			std::vector<std::string> clients;
			for (uint32_t i = 0; i < d->settings.users.size(); ++i)
				if (d->settings.users.at(i).position != UserSettings::notConnected())
					if (d->settings.users.at(i).name != d->localplayername) // all names, but the dedicated server
						clients.push_back(d->settings.users.at(i).name);
			DedicatedLog::get()->game_start(clients, game.map().get_name());
		}
		game.run
			(loaderUI.get(),
			 d->settings.savegame ? Widelands::Game::Loaded : d->settings.scenario ?
			 Widelands::Game::NewMPScenario : Widelands::Game::NewNonScenario,
			 "",
			 false);

		delete tips;

		// if this is an internet game, tell the metaserver that the game is done.
		if (m_internet)
			InternetGaming::ref().set_game_done();

		if (m_is_dedicated) {
			// Statistics: game ended
			std::vector<std::string> winners;
			for (uint32_t i = 0; i < d->settings.users.size(); ++i)
				// We do *not* only check connected users but all, as normally the players are already
				// disconnected once the server reaches this line of code.
				if (d->settings.users.at(i).name != d->localplayername) // all names, but the dedicated server
					if (d->settings.users.at(i).result == Widelands::PlayerEndResult::PLAYER_WON)
						winners.push_back(d->settings.users.at(i).name);
			DedicatedLog::get()->game_end(winners);
		}
		clearComputerPlayers();
	} catch (...) {
		WLApplication::emergency_save(game);
		clearComputerPlayers();
		d->game = nullptr;

		while (!d->clients.empty()) {
			disconnectClient(0, "SERVER_CRASHED");
			reaper();
		}
		throw;
	}
	d->game = nullptr;
}

void NetHost::think()
{
	handle_network();

	if (d->game) {
		int32_t curtime = WLApplication::get()->get_time();
		int32_t delta = curtime - d->lastframe;
		d->lastframe = curtime;

		if (!d->waiting) {
			int32_t diff = (delta * d->networkspeed) / 1000;
			d->pseudo_networktime += diff;
		}

		d->time.think(realSpeed()); // must be called even when d->waiting

		if (d->pseudo_networktime != d->committed_networktime)
		{
			if (d->pseudo_networktime - d->committed_networktime < 0) {
				d->pseudo_networktime = d->committed_networktime;
			} else if (curtime - d->last_heartbeat >= SERVER_TIMESTAMP_INTERVAL) {
				d->last_heartbeat = curtime;

				SendPacket s;
				s.Unsigned8(NETCMD_TIME);
				s.Signed32(d->pseudo_networktime);
				broadcast(s);

				committedNetworkTime(d->pseudo_networktime);

				checkHungClients();
			}
		}

		for (uint32_t i = 0; i < d->computerplayers.size(); ++i)
			d->computerplayers.at(i)->think();
	} else if (m_is_dedicated)
		// Take care that every player gets updated during set up time
		for (uint8_t i = 0; i < d->settings.players.size(); ++i)
			d->npsb.refresh(i);
}

void NetHost::sendPlayerCommand(Widelands::PlayerCommand & pc)
{
	pc.set_duetime(d->committed_networktime + 1);

	SendPacket s;
	s.Unsigned8(NETCMD_PLAYERCOMMAND);
	s.Signed32(pc.duetime());
	pc.serialize(s);
	broadcast(s);
	d->game->enqueue_command(&pc);

	committedNetworkTime(d->committed_networktime + 1);
}

/**
 * All chat messages go through this function.
 * If it is a normal message it is sent to clients as needed, and it is
 * forwarded to our local \ref ChatProvider.
 * If it is a personal message it will only be send to the recipient and to
 * the sender (to show that the message was actually sent).
 */
void NetHost::send(ChatMessage msg)
{
	if (msg.msg.empty())
		return;

	if (isDedicated())
		DedicatedLog::get()->chat(msg);

	if (msg.recipient.empty()) {
		SendPacket s;
		s.Unsigned8(NETCMD_CHAT);
		s.Signed16(msg.playern);
		s.String(msg.sender);
		s.String(msg.msg);
		s.Unsigned8(0);
		broadcast(s);

		d->chat.receive(msg);

		dedicatedlog("[Host]: chat: %s\n", msg.toPlainString().c_str());
	} else { //  personal messages
		SendPacket s;
		s.Unsigned8(NETCMD_CHAT);

		// Is this a pm for the host player?
		if (d->localplayername == msg.recipient) {
			// If this is a dedicated server, handle commands
			if (m_is_dedicated)
				handle_dserver_command(msg.msg, msg.sender);
			d->chat.receive(msg);
			// Write the SendPacket - will be used below to show that the message
			// was received.
			s.Signed16(msg.playern);
			s.String(msg.sender);
			s.String(msg.msg);
			s.Unsigned8(1);
			s.String(msg.recipient);
		} else { //find the recipient
			int32_t clientnum = checkClient(msg.recipient);
			if (clientnum >= 0) {
				s.Signed16(msg.playern);
				s.String(msg.sender);
				s.String(msg.msg);
				s.Unsigned8(1);
				s.String(msg.recipient);
				s.send(d->clients.at(clientnum).sock);
				dedicatedlog("[Host]: personal chat: from %s to %s\n", msg.sender.c_str(), msg.recipient.c_str());
			} else {
				std::string fail = "Failed to send message: Recipient \"";
				fail += msg.recipient + "\" could not be found!";

				// is host the sender?
				if (d->localplayername == msg.sender) {
					ChatMessage err;
					err.playern = -2; // System message
					err.sender = "";
					err.msg = fail;
					err.recipient = "";
					d->chat.receive(err);
					return; // nothing left to do!
				}
				s.Signed16(-2); // System message
				s.String("");
				s.String(fail);
				s.Unsigned8(0);
			}
		}

		if (msg.sender == msg.recipient) //  he sent himself a private message
			return; //  do not deliver it to him twice

		// Now find the sender and send either the message or the failure notice
		else if (msg.playern == -2) // private system message
			return;
		else if (d->localplayername == msg.sender)
			d->chat.receive(msg);
		else { // host is not the sender -> get sender
			if (d->localplayername == msg.recipient && m_is_dedicated)
				return; // There will be an immediate answer from the host
			uint16_t i = 0;
			for (; i < d->settings.users.size(); ++i) {
				const UserSettings & user = d->settings.users.at(i);
				if (user.name == msg.sender)
					break;
			}
			if (i < d->settings.users.size()) {
				uint32_t j = 0;
				for (; j < d->clients.size(); ++j)
					if (d->clients.at(j).usernum == static_cast<int16_t>(i))
						break;
				if (j < d->clients.size())
					s.send(d->clients.at(j).sock);
				else
					// Better no wexception it would break the whole game
					dedicatedlog("WARNING: user was found but no client is connected to it!\n");
			} else
				// Better no wexception it would break the whole game
				dedicatedlog("WARNING: sender could not be found!");
		}
	}
}

/**
 * Checks if client \ref name exists and \returns int32_t :
 *   -   the client number if found
 *   -   -1 if no client was found
 *   -   -2 if the host is the client (has no client number)
 */
int32_t NetHost::checkClient(std::string name)
{
	// Check if the client is the host him-/herself
	if (d->localplayername == name) {
		return -2;
	}

	// Search for the client
	uint16_t i = 0;
	uint32_t client = 0;
	for (; i < d->settings.users.size(); ++i) {
		const UserSettings & user = d->settings.users.at(i);
		if (user.name == name)
			break;
	}
	if (i < d->settings.users.size()) {
		for (; client < d->clients.size(); ++client)
			if (d->clients.at(client).usernum == static_cast<int16_t>(i))
				break;
		if (client >= d->clients.size())
			throw wexception("WARNING: user was found but no client is connected to it!\n");
		return client; // client found
	} else {
		return -1; // no client found
	}
}

/**
* If the host sends a chat message with formation /kick <name> <reason>
* This function will handle this command and try to kick the user.
*/
void NetHost::kickUser(uint32_t client, std::string reason)
{
	disconnectClient(client, "KICKED", true, reason);
}


/// Split up a user entered string in "cmd", "arg1" and "arg2"
/// \note the cmd must begin with "/"
void NetHost::splitCommandArray
	(const std::string & cmdarray, std::string & cmd, std::string & arg1, std::string & arg2)
{
	assert(cmdarray.size() > 1);

	std::string::size_type const space = cmdarray.find(' ');
	if (space > cmdarray.size())
		// only cmd
		cmd = cmdarray.substr(0);
	else {
		cmd = cmdarray.substr(0, space);
		std::string::size_type const space2 = cmdarray.find(' ', space + 1);
		if (space2 != std::string::npos) {
			// cmd + arg1 + arg2
			arg1 = cmdarray.substr(space + 1, space2 - space - 1);
			arg2 = cmdarray.substr(space2 + 1);
		} else if (space + 1 < cmdarray.size())
			// cmd + arg1
			arg1 = cmdarray.substr(space + 1);
	}
	if (arg1.empty())
		arg1 = "";
	if (arg2.empty())
		arg2 = "";
}


/**
 * This function is used to handle commands for the dedicated server
 */
void NetHost::handle_dserver_command(std::string cmdarray, std::string sender)
{
	assert(m_is_dedicated);

	ChatMessage c;
	c.time = time(nullptr);
	c.playern = -2;
	c.sender = d->localplayername;
	c.recipient = sender;

	// Find the client that send the chat message
	int32_t num = checkClient(sender);
	if (num < 0) // host or not found
		return;
	Client & client = d->clients[num];

	if (cmdarray.size() < 1) {
		return;
	}

	// Split up in "cmd" "arg1" "arg2"
	std::string cmd, arg1, arg2;
	splitCommandArray(cmdarray, cmd, arg1, arg2);

	// help
	if (cmd == "help") {
		if (d->game)
			c.msg =
				(boost::format("<br>%s<br>%s<br>%s<br>%s")
					% _("Available host commands are:")
					/** TRANSLATORS: Available host command */
					% _("/help  -  Shows this help")
					/** TRANSLATORS: Available host command */
					% _("host $ - Tries to run the host command $")
					/** TRANSLATORS: Available host command */
					% _("save $ - Saves the current game state as $.wgf")
				).str();
		else
			c.msg =
				(boost::format("<br>%s<br>%s<br>%s")
					% _("Available host commands are:")
					/** TRANSLATORS: Available host command */
					% _("/help  -  Shows this help")
					/** TRANSLATORS: Available host command */
					% _("host $ - Tries to run the host command $")
				).str();
		if (m_password.size() > 1) {
			c.msg += "<br>";
			c.msg += _("pwd $  - Sends the password $ to the host");
		}
		send(c);

		// host
	} else if (cmd == "host") {
		if (!client.dedicated_access) {
			c.msg = _("Access to host commands denied. To gain access, send the password with pwd command.");
			send(c);
			return;
		}
		std::string temp = arg1 + " " + arg2;
		c.msg = (format(_("%1$s told me to run the command: \"%2$s\"")) % sender % temp).str();
		c.recipient = "";
		send(c);
		d->chat.send(temp);

	} else if (cmd == "save") {
		// Check whether saving is allowed at all
		Section & s = g_options.pull_section("global");
		if (!s.get_bool("dedicated_saving", true)) {
			c.msg = _("Sorry! Saving was deactivated on this dedicated server!");
			send(c);
		} else if (!d->game) {
			c.msg = _("Cannot save while there is no game running!");
			send(c);
		} else {
			//try to save the game
			std::string savename =  "save/" + arg1;
			if (arg2.size() > 0) // only add space and arg2, if arg2 has anything to print.
				savename += " " + arg2;
			savename += ".wgf";
			std::string * error = new std::string();
			SaveHandler & sh = d->game->save_handler();
			if (sh.save_game(*d->game, savename, error))
				c.msg = _("Game successfully saved!");
			else
				c.msg =
					(format(_("Could not save the game to the file \"%1$s\"! (%2$s)"))
					 % savename % error)
					 .str();
			send(c);
			delete error;
		}

	} else if (cmd == "pwd") {
		if (m_password.size() == 0) {
			c.msg = _("This server is not password protected!");
			send(c);
		} else if (arg1 != m_password) {
			c.msg = _("The sent password was incorrect!");
			send(c);
		} else {
			// Once the client gained access (s)he might need the knowledge about available maps and saved games
			dserver_send_maps_and_saves(client);

			// Send the client the access granted message
			SendPacket s;
			s.reset();
			s.Unsigned8(NETCMD_DEDICATED_ACCESS);
			s.send(client.sock);
			client.dedicated_access = true;

			c.msg = _("The password was correct, access was granted!");
			send(c);
		}

	// default
	} else {
		c.msg = (format(_("Unknown dedicated server command \"%s\"!")) % cmd).str();
		send(c);
	}
}

void NetHost::dserver_send_maps_and_saves(Client & client) {
	assert (not d->game);

	if (d->settings.maps.empty()) {
		// Read in maps
		std::vector<std::string> directories;
		directories.push_back("maps");
		while (!directories.empty()) {
			filenameset_t files = g_fs->ListDirectory(directories.at(directories.size() - 1).c_str());
			directories.resize(directories.size() - 1);
			Widelands::Map map;
			const filenameset_t & gamefiles = files;
			container_iterate_const(filenameset_t, gamefiles, i) {
				char const * const name = i.current->c_str();
				std::unique_ptr<Widelands::Map_Loader> ml = map.get_correct_loader(name);
				if (ml) {
					map.set_filename(name);
					ml->preload_map(true);
					DedicatedMapInfos info;
					info.path     = name;
					info.players  = map.get_nrplayers();
					info.scenario = map.scenario_types() & Widelands::Map::MP_SCENARIO;
					d->settings.maps.push_back(info);
				} else {
					if
						(g_fs->IsDirectory(name)
						&&
						strcmp(FileSystem::FS_Filename(name), ".")
						&&
						strcmp(FileSystem::FS_Filename(name), ".."))
					{
						directories.push_back(name);
					}
				}
			}
		}
	}

	if (d->settings.saved_games.empty()) {
		// Read in saved games
		filenameset_t files = g_fs->ListDirectory("save");
		Widelands::Game game;
		Widelands::Game_Preload_Data_Packet gpdp;
		const filenameset_t & gamefiles = files;
		container_iterate_const(filenameset_t, gamefiles, i) {
			char const * const name = i.current->c_str();
			try {
				Widelands::Game_Loader gl(name, game);
				gl.preload_game(gpdp);

				// If we are here, the saved game is valid
				std::unique_ptr<FileSystem> sg_fs(g_fs->MakeSubFileSystem(name));
				Profile prof;
				prof.read("map/elemental", nullptr, *sg_fs);
				Section & s = prof.get_safe_section("global");

				DedicatedMapInfos info;
				info.path     = name;
				info.players  = static_cast<uint8_t>(s.get_safe_int("nr_players"));
				d->settings.saved_games.push_back(info);
			} catch (const _wexception &) {}
		}
	}

	SendPacket s;

	// Send list of maps
	for (uint8_t i = 0; i < d->settings.maps.size(); ++i) {
		s.reset();
		s.Unsigned8(NETCMD_DEDICATED_MAPS);
		s.String   (d->settings.maps[i].path);
		s.Unsigned8(d->settings.maps[i].players);
		s.Unsigned8(d->settings.maps[i].scenario ? 1 : 0);
		s.send(client.sock);
	}

	// Send list of saved games
	for (uint8_t i = 0; i < d->settings.saved_games.size(); ++i) {
		s.reset();
		s.Unsigned8(NETCMD_DEDICATED_SAVED_GAMES);
		s.String   (d->settings.saved_games[i].path);
		s.Unsigned8(d->settings.saved_games[i].players);
		s.send(client.sock);
	}
}

void NetHost::sendSystemMessageCode
	(const std::string & code, const std::string & a, const std::string & b, const std::string & c)
{
	// First send to all clients
	SendPacket s;
	s.Unsigned8(NETCMD_SYSTEM_MESSAGE_CODE);
	s.String(code);
	s.String(a);
	s.String(b);
	s.String(c);
	broadcast(s);

	// Now add to our own chatbox
	ChatMessage msg;
	msg.time = time(nullptr);
	msg.msg = NetworkGamingMessages::get_message(code, a, b, c);
	msg.playern = UserSettings::none(); //  == System message
	// c.sender remains empty to indicate a system message
	d->chat.receive(msg);
	if (m_is_dedicated)
		DedicatedLog::get()->chat(msg);
}

int32_t NetHost::getFrametime()
{
	return d->time.time() - d->game->get_gametime();
}

std::string NetHost::getGameDescription()
{
	char buf[200];
	snprintf
		(buf, sizeof(buf),
		 "network player %i (host)", d->settings.users.at(0).position);
	return buf;
}

const GameSettings& NetHost::settings()
{
	return d->settings;
}

bool NetHost::canLaunch()
{
	if (d->settings.mapname.empty())
		return false;
	if (d->settings.players.size() < 1)
		return false;
	if (d->game)
		return false;

	// if there is one client that is currently receiving a file, we can not launch.
	for (std::vector<Client>::iterator j = d->clients.begin(); j != d->clients.end(); ++j) {
		if (!d->settings.users[j->usernum].ready)
			return false;
	}

	// all players must be connected to a controller (human/ai) or be closed.
	// but not all should be closed!
	bool one_not_closed = false;
	for (size_t i = 0; i < d->settings.players.size(); ++i) {
		if (d->settings.players.at(i).state != PlayerSettings::stateClosed)
			one_not_closed = true;
		if (d->settings.players.at(i).state == PlayerSettings::stateOpen)
			return false;
	}
	return one_not_closed;
}

void NetHost::setMap
	(const std::string &       mapname,
	 const std::string &       mapfilename,
	 uint32_t            const maxplayers,
	 bool                const savegame)
{
	d->settings.mapname = mapname;
	d->settings.mapfilename = mapfilename;
	d->settings.savegame = savegame;

	std::vector<PlayerSettings>::size_type oldplayers = d->settings.players.size();

	SendPacket s;

	// Care about the host
	if
		(static_cast<int32_t>(maxplayers) <= d->settings.playernum
		 &&
		 d->settings.playernum != UserSettings::none())
	{
		setPlayerNumber(UserSettings::none());
	}

	while (oldplayers > maxplayers) {
		--oldplayers;
		for (uint16_t i = 1; i < d->settings.users.size(); ++i)
			if (d->settings.users.at(i).position == oldplayers) {
				d->settings.users.at(i).position = UserSettings::none();

				// for local settings
				uint32_t j = 0;
				for (; j < d->clients.size(); ++j)
					if (d->clients.at(j).usernum == static_cast<int16_t>(i))
						break;
				d->clients.at(j).playernum = UserSettings::none();

				// Broadcast change
				s.reset();
				s.Unsigned8(NETCMD_SETTING_USER);
				s.Unsigned32(i);
				writeSettingUser(s, i);
				broadcast(s);
			}
	}

	d->settings.players.resize(maxplayers);

	while (oldplayers < maxplayers) {
		PlayerSettings & player = d->settings.players.at(oldplayers);
		player.state                = PlayerSettings::stateOpen;
		player.name                 = "";
		player.tribe                = d->settings.tribes.at(0).name;
		player.random_tribe         = false;
		player.initialization_index = 0;
		player.team                 = 0;
		player.ai                   = "";
		player.random_ai            = false;
		player.closeable            = false;
		player.shared_in            = 0;

		++oldplayers;
	}

	// Broadcast new map info
	s.reset();
	s.Unsigned8(NETCMD_SETTING_MAP);
	writeSettingMap(s);
	broadcast(s);

	// Broadcast new player settings
	s.reset();
	s.Unsigned8(NETCMD_SETTING_ALLPLAYERS);
	writeSettingAllPlayers(s);
	broadcast(s);

	// If possible, offer the map / saved game as transfer
	// TODO not yet able to handle directory type maps / savegames
	if (!g_fs->IsDirectory(mapfilename)) {
		// Read in the file
		FileRead fr;
		fr.Open(*g_fs, mapfilename.c_str());
		if (file)
			delete file;
		file = new NetTransferFile();
		file->filename = mapfilename;
		uint32_t leftparts = file->bytes = fr.GetSize();
		while (leftparts > 0) {
			uint32_t readout = (leftparts > NETFILEPARTSIZE) ? NETFILEPARTSIZE : leftparts;
			FilePart fp;
			memcpy(fp.part, fr.Data(readout), readout);
			file->parts.push_back(fp);
			leftparts -= readout;
		}
		std::vector<char> complete(file->bytes);
		fr.SetFilePos(0);
		fr.DataComplete(&complete[0], file->bytes);
		SimpleMD5Checksum md5sum;
		md5sum.Data(&complete[0], file->bytes);
		md5sum.FinishChecksum();
		file->md5sum = md5sum.GetChecksum().str();
	} else {
		// reset previously offered map / saved game
		if (file) {
			delete file;
			file = nullptr;
		}
	}

	s.reset();
	if (writeMapTransferInfo(s, mapfilename))
		broadcast(s);
}

void NetHost::setPlayerState
	(uint8_t const number, PlayerSettings::State const state, bool const host)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players.at(number);

	if (player.state == state)
		return;

	SendPacket s;

	if (player.state == PlayerSettings::stateHuman)
		//  0 is host and has no client
		if (d->settings.users.at(0).position == number) {
			d->settings.users.at(0).position = UserSettings::none();
			d->settings.playernum = UserSettings::none();
		}
		for (uint8_t i = 1; i < d->settings.users.size(); ++i)
			if (d->settings.users.at(i).position == number) {
				d->settings.users.at(i).position = UserSettings::none();
				if (host) //  Did host send the user to lobby?
					sendSystemMessageCode("SENT_PLAYER_TO_LOBBY", d->settings.users.at(i).name);

				//  for local settings
				for (std::vector<Client>::iterator j = d->clients.begin();; ++j) {
					assert(j != d->clients.end());
					if (j->usernum == i) {
						j->playernum = UserSettings::none();
						break;
					}
				}

				//  broadcast change
				s.Unsigned8(NETCMD_SETTING_USER);
				s.Unsigned32(i);
				writeSettingUser(s, i);
				broadcast(s);

				break;
			}

	player.state = state;

	if (player.state == PlayerSettings::stateComputer)
		player.name = getComputerPlayerName(number);

	// Broadcast change
	s.reset();
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}


void NetHost::setPlayerTribe(uint8_t const number, const std::string & tribe, bool const random_tribe)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players.at(number);

	if (player.tribe == tribe && player.random_tribe == random_tribe)
		return;

	std::string actual_tribe = tribe;
	player.random_tribe = random_tribe;

	if (random_tribe) {
		uint8_t num_tribes = d->settings.tribes.size();
		uint8_t random = (std::rand() % num_tribes);
		actual_tribe = d->settings.tribes.at(random).name;
	}

	container_iterate_const(std::vector<TribeBasicInfo>, d->settings.tribes, i)
		if (i.current->name == player.tribe) {
			player.tribe = actual_tribe;
			if (i.current->initializations.size() <= player.initialization_index)
				player.initialization_index = 0;

			//  broadcast changes
			SendPacket s;
			s.Unsigned8(NETCMD_SETTING_PLAYER);
			s.Unsigned8(number);
			writeSettingPlayer(s, number);
			broadcast(s);
			return;
		}
	log
		("Player %u attempted to change to tribe %s; not a valid tribe\n",
		 number, tribe.c_str());
}

void NetHost::setPlayerInit(uint8_t const number, uint8_t const index)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players.at(number);

	if (player.initialization_index == index)
		return;

	container_iterate_const(std::vector<TribeBasicInfo>, d->settings.tribes, i)
		if (i.current->name == player.tribe) {
			if (index < i.current->initializations.size()) {
				player.initialization_index = index;

				//  broadcast changes
				SendPacket s;
				s.Unsigned8(NETCMD_SETTING_PLAYER);
				s.Unsigned8(number);
				writeSettingPlayer(s, number);
				broadcast(s);
				return;
			} else
				log
					("Attempted to change to out-of-range initialization index %u "
					 "for player %u.\n", index, number);
			return;
		}
	assert(false);
}


void NetHost::setPlayerAI(uint8_t number, const std::string & name, bool const random_ai)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players.at(number);
	player.ai = name;
	player.random_ai = random_ai;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}


void NetHost::setPlayerName(uint8_t const number, const std::string & name)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players.at(number);

	if (player.name == name)
		return;

	player.name = name;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}


void NetHost::setPlayerCloseable(uint8_t const number, bool closeable)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players.at(number);

	if (player.closeable == closeable)
		return;

	player.closeable = closeable;

	// There is no need to broadcast a player closeability change, as the host is the only one who uses it.
}


void NetHost::setPlayerShared(uint8_t number, uint8_t shared) {
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players.at(number);

	if (player.shared_in == shared)
		return;

	PlayerSettings & sharedplr = d->settings.players.at(shared - 1);
	assert(sharedplr.state != PlayerSettings::stateClosed && sharedplr.state != PlayerSettings::stateShared);

	player.shared_in = shared;
	player.tribe     = sharedplr.tribe;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}


void NetHost::setPlayer(uint8_t const number, PlayerSettings const ps)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players.at(number);
	player = ps;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}

void NetHost::setPlayerNumber(uint8_t const number)
{
	switchToPlayer(0, number);
}

void NetHost::setWinConditionScript(std::string wc)
{
	d->settings.win_condition_script = wc;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_WIN_CONDITION);
	s.String(wc);
	broadcast(s);
}

void NetHost::switchToPlayer(uint32_t user, uint8_t number)
{
	if
		(number < d->settings.players.size()
		 &&
		 (d->settings.players.at(number).state != PlayerSettings::stateOpen
		  &&
		  d->settings.players.at(number).state != PlayerSettings::stateHuman))
		return;

	uint32_t old = d->settings.users.at(user).position;
	std::string name = d->settings.users.at(user).name;
	// Remove clients name from old player slot
	if (old < d->settings.players.size()) {
		PlayerSettings & op = d->settings.players.at(old);
		std::string temp(" ");
		temp += name;
		temp += " ";
		std::string temp2(op.name);
		temp2 = temp2.erase(op.name.find(temp), temp.size());
		setPlayerName(old, temp2);
		if (temp2.empty())
			setPlayerState(old, PlayerSettings::stateOpen);
	}

	if (number < d->settings.players.size()) {
		// Add clients name to new player slot
		PlayerSettings & op = d->settings.players.at(number);
		if (op.state == PlayerSettings::stateOpen) {
			setPlayerState(number, PlayerSettings::stateHuman);
			setPlayerName(number, " " + name + " ");
		} else
			setPlayerName(number, op.name + " " + name + " ");
	}
	d->settings.users.at(user).position = number;
	if (user == 0) // host
		d->settings.playernum = number;
	else
		for (uint32_t j = 0; j < d->clients.size(); ++j)
			if (d->clients.at(j).usernum == static_cast<int16_t>(user)) {
				d->clients.at(j).playernum = number;
				break;
			}

	// Broadcast the user changes to everybody
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_USER);
	s.Unsigned32(user);
	writeSettingUser(s, user);
	broadcast(s);
}

void NetHost::setPlayerTeam(uint8_t number, Widelands::TeamNumber team)
{
	if (number >= d->settings.players.size())
		return;
	d->settings.players.at(number).team = team;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}

void NetHost::setMultiplayerGameSettings()
{
	d->settings.scenario = false;
	d->settings.multiplayer = true;
}

void NetHost::setScenario(bool is_scenario)
{
	d->settings.scenario = is_scenario;
}

uint32_t NetHost::realSpeed()
{
	if (d->waiting)
		return 0;
	return d->networkspeed;
}

uint32_t NetHost::desiredSpeed()
{
	return d->localdesiredspeed;
}

void NetHost::setDesiredSpeed(uint32_t const speed)
{
	if (speed != d->localdesiredspeed) {
		d->localdesiredspeed = speed;
		updateNetworkSpeed();
	}
}

// Network games cannot be paused
bool NetHost::isPaused()
{
	return false;
}

void NetHost::setPaused(bool /* paused */)
{
}

// Send the packet to all properly connected clients
void NetHost::broadcast(SendPacket & packet)
{
	container_iterate_const(std::vector<Client>, d->clients, i)
		if (i.current->playernum != UserSettings::notConnected())
			packet.send(i.current->sock);
}

void NetHost::writeSettingMap(SendPacket & packet)
{
	packet.String(d->settings.mapname);
	packet.String(d->settings.mapfilename);
	packet.Unsigned8(d->settings.savegame ? 1 : 0);
	packet.Unsigned8(d->settings.scenario ? 1 : 0);
}

void NetHost::writeSettingPlayer(SendPacket & packet, uint8_t const number)
{
	PlayerSettings & player = d->settings.players.at(number);
	packet.Unsigned8(static_cast<uint8_t>(player.state));
	packet.String(player.name);
	packet.String(player.tribe);
	packet.Unsigned8(player.random_tribe ? 1 : 0);
	packet.Unsigned8(player.initialization_index);
	packet.String(player.ai);
	packet.Unsigned8(player.random_ai ? 1 : 0);
	packet.Unsigned8(player.team);
	packet.Unsigned8(player.shared_in);
}

void NetHost::writeSettingAllPlayers(SendPacket & packet)
{
	packet.Unsigned8(d->settings.players.size());
	for (uint8_t i = 0; i < d->settings.players.size(); ++i)
		writeSettingPlayer(packet, i);
}

void NetHost::writeSettingUser(SendPacket & packet, uint32_t const number)
{
	packet.String(d->settings.users.at(number).name);
	packet.Signed32(d->settings.users.at(number).position);
	packet.Unsigned8(d->settings.users.at(number).ready ? 1 : 0);
}

void NetHost::writeSettingAllUsers(SendPacket & packet)
{
	packet.Unsigned8(d->settings.users.size());
	for (uint32_t i = 0; i < d->settings.users.size(); ++i)
		writeSettingUser(packet, i);
}


/**
* If possible, this function writes the MapTransferInfo to SendPacket & s
*
* \returns true if the data was written, else false
*/
bool NetHost::writeMapTransferInfo(SendPacket & s, std::string mapfilename) {
	// TODO not yet able to handle directory type maps / savegames
	if (g_fs->IsDirectory(mapfilename)) {
		dedicatedlog("Map/Save is a directory! No way for making it available a.t.m.!\n");
		return false;
	}

	// Write the new map/save file information, so client can decide whether it
	// needs the file.
	s.Unsigned8(NETCMD_NEW_FILE_AVAILABLE);
	s.String(mapfilename);
	s.Unsigned32(file->bytes);
	s.String(file->md5sum);
	return true;
}


/**
 *
 * \return a name for the given player.
 */
std::string NetHost::getComputerPlayerName(uint8_t const playernum)
{
	std::string name;
	uint32_t suffix = playernum;
	do {
		char buf[200];
		snprintf(buf, sizeof(buf), _("Computer %u"), ++suffix);
		name = buf;
	} while (haveUserName(name, playernum));
	return name;
}


/**
 * Checks whether a user with the given name exists already.
 *
 * If \p ignoreplayer < UserSettings::highestPlayernum(), the user with this
 * number will be ignored.
 */
bool NetHost::haveUserName(const std::string & name, uint8_t ignoreplayer) {
	for (uint32_t i = 0; i < d->settings.users.size(); ++i)
		if (i != ignoreplayer and d->settings.users.at(i).name == name)
			return true;

	// Computer players are not handled like human users,
	// so make sure no cp owns this name.
	if (ignoreplayer < d->settings.users.size())
		ignoreplayer = d->settings.users.at(ignoreplayer).position;
	for (uint32_t i = 0; i < d->settings.players.size(); ++i)
		if (i != ignoreplayer and d->settings.players.at(i).name == name)
			return true;

	return false;
}


/// Respond to a client's Hello message.
void NetHost::welcomeClient (uint32_t const number, std::string & playername)
{
	assert(number < d->clients.size());

	Client & client = d->clients.at(number);

	assert(client.playernum == UserSettings::notConnected());
	assert(client.sock);

	// Just for statistics
	DedicatedLog::get()->client_login();

	// The client gets its own initial data set.
	client.playernum = UserSettings::none();
	// only used at password protected dedicated server, but better initialize always
	client.dedicated_access = m_is_dedicated ? (m_password.size() == 0) : false;

	if (!d->game) // just in case we allow connection of spectators/players after game start
		for (uint32_t i = 0; i < d->settings.users.size(); ++i)
			if (d->settings.users[i].position == UserSettings::notConnected()) {
				client.usernum = i;
				d->settings.users[i].result = Widelands::PlayerEndResult::UNDEFINED;
				d->settings.users[i].ready  = true;
				break;
			}
	if (client.usernum == -1) {
		client.usernum = d->settings.users.size();
		UserSettings newuser;
		newuser.result = Widelands::PlayerEndResult::UNDEFINED;
		newuser.ready  = true;
		d->settings.users.push_back(newuser);
	}

	// Assign the player a name, preferably the name chosen by the client
	if (playername.empty()) // Make sure there is at least a name base.
		playername = _("Player");
	std::string effective_name = playername;

	if (haveUserName(effective_name, client.usernum)) {
		uint32_t i = 2;
		do {
			char buf[32];
			snprintf(buf, sizeof(buf), "%u", i++);
			effective_name = (boost::format(_("Player %s")) % buf).str();
		} while (haveUserName(effective_name, client.usernum));
	}

	d->settings.users.at(client.usernum).name = effective_name;
	d->settings.users.at(client.usernum).position = UserSettings::none();

	dedicatedlog("[Host]: Client %u: welcome to usernum %u\n", number, client.usernum);

	SendPacket s;
	s.Unsigned8(NETCMD_HELLO);
	s.Unsigned8(NETWORK_PROTOCOL_VERSION);
	s.Unsigned32(client.usernum);
	s.send(client.sock);

	// even if the network protocol is the same, the data might be different.
	if (client.build_id != build_id())
		sendSystemMessageCode("DIFFERENT_WL_VERSION", effective_name, client.build_id, build_id());

	// Send information about currently selected map / savegame
	s.reset();
	s.Unsigned8(NETCMD_SETTING_MAP);
	writeSettingMap(s);
	s.send(client.sock);

	// If possible, offer the map / savegame as transfer
	if (file) {
		s.reset();
		if (writeMapTransferInfo(s, file->filename))
			s.send(client.sock);
	}

	//  Send the tribe information to the new client.
	s.reset();
	s.Unsigned8(NETCMD_SETTING_TRIBES);
	s.Unsigned8(d->settings.tribes.size());
	for (uint8_t i = 0; i < d->settings.tribes.size(); ++i) {
		s.String(d->settings.tribes[i].name);
		size_t const nr_initializations =
			d->settings.tribes[i].initializations.size();
		s.Unsigned8(nr_initializations);
		for (uint8_t j = 0; j < nr_initializations; ++j)
			s.String(d->settings.tribes[i].initializations[j].first);
	}
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_SETTING_ALLPLAYERS);
	writeSettingAllPlayers(s);
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_SETTING_ALLUSERS);
	writeSettingAllUsers(s);
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_WIN_CONDITION);
	s.String(d->settings.win_condition_script);
	s.send(client.sock);

	// Broadcast new information about the player to everybody
	s.reset();
	s.Unsigned8(NETCMD_SETTING_USER);
	s.Unsigned32(client.usernum);
	writeSettingUser(s, client.usernum);
	broadcast(s);

	// Check if there is an unoccupied player left and if, assign.
	for (uint8_t i = 0; i < d->settings.players.size(); ++i)
		if (d->settings.players.at(i).state == PlayerSettings::stateOpen) {
			switchToPlayer(client.usernum, i);
			break;
		}

	sendSystemMessageCode("CLIENT_HAS_JOINED_GAME", effective_name);

	// If this is a dedicated server, inform the player
	if (m_is_dedicated) {
		ChatMessage c;
		c.time = time(nullptr);
		c.playern = -2;
		c.sender = d->localplayername;
		// Send the message of the day if exists
		c.msg = "<br>" + m_dedicated_motd;
		if (m_password.size() > 1) {
			c.msg += "<br>";
			c.msg +=
				(format
					(_("This server is password protected. You can send the password with: \"@%s pwd PASSWORD\""))
					% d->localplayername)
				.str();
		} else {
			// Once the client gained access it might need the knowledge about available maps and saved games
			dserver_send_maps_and_saves(client);

			// If not password protected, give the client access to the settings
			s.reset();
			s.Unsigned8(NETCMD_DEDICATED_ACCESS);
			s.send(client.sock);
		}
		c.recipient = d->settings.users.at(client.usernum).name;
		send(c);
	}
}

void NetHost::committedNetworkTime(int32_t const time)
{
	assert(time - d->committed_networktime > 0);

	d->committed_networktime = time;
	d->time.recv(time);

	if
		(!d->syncreport_pending &&
		 d->committed_networktime - d->syncreport_time >= SYNCREPORT_INTERVAL)
		requestSyncReports();
}

void NetHost::recvClientTime(uint32_t const number, int32_t const time)
{
	assert(number < d->clients.size());

	Client & client = d->clients.at(number);

	if (time - client.time < 0)
		throw DisconnectException("BACKWARTS_RUNNING_TIME");
	if (d->committed_networktime - time < 0)
		throw DisconnectException("SIMULATING_BEYOND_TIME");
	if (d->syncreport_pending && !client.syncreport_arrived) {
		if (time - d->syncreport_time > 0)
			throw DisconnectException("CLIENT_SYNC_REP_TIMEOUT");
	}

	client.time = time;
	dedicatedlog("[Host]: Client %i: Time %i\n", number, time);

	if (d->waiting) {
		log
			("[Host]: Client %i reports time %i (networktime = %i) during hang\n",
			 number, time, d->committed_networktime);
		checkHungClients();
	}
}


void NetHost::checkHungClients()
{
	assert(d->game != nullptr);

	int nrready = 0;
	int nrdelayed = 0;
	int nrhung = 0;

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients.at(i).playernum == UserSettings::notConnected())
			continue;

		int32_t const delta = d->committed_networktime - d->clients.at(i).time;

		if (delta == 0) {
			++nrready;
			// reset the hung_since time
			d->clients.at(i).hung_since = 0;
		} else {
			assert(d->game != nullptr);
			++nrdelayed;
			if
				(delta
				 >
				 (5                         *
				  CLIENT_TIMESTAMP_INTERVAL *
				  static_cast<int32_t>(d->networkspeed))
				 /
				 1000)
			{
				log
					("[Host]: Client %i (%s) hung\n",
					 i, d->settings.users.at(d->clients.at(i).usernum).name.c_str());
				++nrhung;
				if (d->clients.at(i).hung_since == 0) {
					d->clients.at(i).hung_since = time(nullptr);
					d->clients.at(i).lastdelta = 0;
				} else if (time_t deltanow = time(nullptr) - d->clients.at(i).hung_since > 60) {

					// inform the other clients about the problem regulary
					if (deltanow - d->clients.at(i).lastdelta > 30) {
						char buf[5];
						//snprintf(buf, sizeof(buf), "%li", deltanow);
						snprintf(buf, sizeof(buf), ngettext("%li second", "%li seconds", deltanow), deltanow);
						sendSystemMessageCode
							("CLIENT_HUNG", d->settings.users.at(d->clients.at(i).usernum).name, buf);
						d->clients.at(i).lastdelta = deltanow;
						if (m_is_dedicated) {
							snprintf(buf, sizeof(buf), "%li", 300 - deltanow);
							sendSystemMessageCode
								("CLIENT_HUNG_AUTOKICK", d->settings.users.at(d->clients.at(i).usernum).name, buf);
						}
					}

					// If this is a dedicated server, there is no host that cares about kicking hung players
					// This is especially problematic, if the last or all players hung and the dedicated
					// server does not automatically restart.
					// 5 minutes for all other players to react before the dedicated server takes care
					// about the situation itself
					if ((d->clients.at(i).hung_since < (time(nullptr) - 300)) && m_is_dedicated) {
						disconnectClient(i, "CLIENT_TIMEOUTED");
						// Try to save the game
						std::string savename = (boost::format("save/client_hung_%i.wmf") % time(nullptr)).str();;
						std::string * error = new std::string();
						SaveHandler & sh = d->game->save_handler();
						if (sh.save_game(*d->game, savename, error))
							sendSystemMessageCode("GAME_SAVED_AS", savename);
						delete error;
					}
				}
			}
		}
	}

	if (!d->waiting) {
		if (nrhung) {
			dedicatedlog("[Host]: %i clients hung. Entering wait mode\n", nrhung);

			// Brake and wait
			d->waiting = true;
			broadcastRealSpeed(0);

			SendPacket s;
			s.Unsigned8(NETCMD_WAIT);
			broadcast(s);
		}
	} else {
		if (nrdelayed == 0) {
			d->waiting = false;
			broadcastRealSpeed(d->networkspeed);
			if (!d->syncreport_pending)
				requestSyncReports();
		}
	}
}


void NetHost::broadcastRealSpeed(uint32_t const speed)
{
	assert(speed <= std::numeric_limits<uint16_t>::max());

	SendPacket s;
	s.Unsigned8(NETCMD_SETSPEED);
	s.Unsigned16(speed);
	broadcast(s);
}


/**
 * This is the algorithm that decides upon the effective network speed,
 * given the desired speed of all clients.
 *
 * This function is supposed to be the only code that ever changes
 * \ref NetHostImpl::networkspeed.
 *
 * The current implementation picks the median, or the average of
 * lower and upper median.
 * If only two players are playing (host + 1 client), there is a boundary
 * so neither the host, nor the players can abuse their setting to get to the
 * wished speed (e.g. without this boundary, the client might set his/her wished
 *               speed to 8x - even if the host sets his desired speed to PAUSE
 *               the median sped would be 4x).
 *
 * The immediate pausing (with the Pause key) is disabled completely in the
 * network games, as sudden pauses would be distracting to other players. A
 * hard interruption of the game can be achieved with the forced pause.
 */
void NetHost::updateNetworkSpeed()
{
	uint32_t const oldnetworkspeed = d->networkspeed;

	// First check if a pause was forced by the host
	if (m_forced_pause)
		d->networkspeed = 0;

	else {
		// No pause was forced - normal speed calculation
		std::vector<uint32_t> speeds;

		if (!m_is_dedicated)
			speeds.push_back(d->localdesiredspeed);
		for (uint32_t i = 0; i < d->clients.size(); ++i) {
			if (d->clients.at(i).playernum <= UserSettings::highestPlayernum())
				speeds.push_back(d->clients.at(i).desiredspeed);
		}
		if (speeds.empty()) // Possible in dedicated server games with only spectators
			return;

		std::sort(speeds.begin(), speeds.end());

		// Abuse prevention for 2 players
		if (speeds.size() == 2) {
			if (speeds[0] > speeds[1] + 1000)
				speeds[0] = speeds[1] + 1000;
			if (speeds[1] > speeds[0] + 1000)
				speeds[1] = speeds[0] + 1000;
		}


		d->networkspeed =
			speeds.size() % 2 ? speeds.at(speeds.size() / 2) :
			(speeds.at(speeds.size() / 2) + speeds.at((speeds.size() / 2) - 1))
			/ 2;

		if (d->networkspeed > std::numeric_limits<uint16_t>::max())
			d->networkspeed = std::numeric_limits<uint16_t>::max();
	}

	if (d->networkspeed != oldnetworkspeed && !d->waiting)
		broadcastRealSpeed(d->networkspeed);
}


/**
 * Request sync reports from all clients at the next possible time.
 */
void NetHost::requestSyncReports()
{
	assert(!d->syncreport_pending);

	d->syncreport_pending = true;
	d->syncreport_arrived = false;
	d->syncreport_time = d->committed_networktime + 1;

	for (uint32_t i = 0; i < d->clients.size(); ++i)
		d->clients.at(i).syncreport_arrived = false;

	dedicatedlog("[Host]: Requesting sync reports for time %i\n", d->syncreport_time);

	SendPacket s;
	s.Unsigned8(NETCMD_SYNCREQUEST);
	s.Signed32(d->syncreport_time);
	broadcast(s);

	d->game->enqueue_command(new Cmd_NetCheckSync(d->syncreport_time, this));

	committedNetworkTime(d->syncreport_time);
}

/**
 * Check whether all sync reports have arrived, and if so, compare.
 */
void NetHost::checkSyncReports()
{
	assert(d->syncreport_pending);

	if (!d->syncreport_arrived)
		return;

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if
			(d->clients.at(i).playernum != UserSettings::notConnected() &&
			 !d->clients.at(i).syncreport_arrived)
			return;
	}

	d->syncreport_pending = false;
	dedicatedlog("[Host]: comparing syncreports for time %i\n", d->syncreport_time);

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		Client & client = d->clients.at(i);
		if (client.playernum == UserSettings::notConnected())
			continue;

		if (client.syncreport != d->syncreport) {
			log
				("[Host]: lost synchronization with client %u!\n"
				 "I have:     %s\n"
				 "Client has: %s\n",
				 i, d->syncreport.str().c_str(), client.syncreport.str().c_str());

			d->game->save_syncstream(true);

			SendPacket s;
			s.Unsigned8(NETCMD_INFO_DESYNC);
			broadcast(s);

			disconnectClient(i, "CLIENT_DESYNCED");
			// Pause the game, so that host and client have time to handle the
			// desync.
			d->networkspeed = 0;
			broadcastRealSpeed(d->networkspeed);
		}
	}
}

void NetHost::syncreport()
{
	assert(d->game->get_gametime() == d->syncreport_time);

	d->syncreport = d->game->get_sync_hash();
	d->syncreport_arrived = true;

	checkSyncReports();
}


void NetHost::handle_network ()
{
	TCPsocket sock;

	if (d->promoter != nullptr)
		d->promoter->run ();

	// Check for new connections.
	while (d->svsock != nullptr && (sock = SDLNet_TCP_Accept(d->svsock)) != nullptr) {
		dedicatedlog("[Host]: Received a connection request\n");

		SDLNet_TCP_AddSocket (d->sockset, sock);

		Client peer;

		peer.sock               = sock;
		peer.playernum          = UserSettings::notConnected();
		peer.syncreport_arrived = false;
		peer.desiredspeed       = 1000;
		peer.usernum            = -1; // == no user assigned for now.
		peer.hung_since         = 0;
		peer.lastdelta          = 0;
		d->clients.push_back(peer);
	}

	// if this is an internet game, handle the metaserver information
	if (m_internet) {
		InternetGaming::ref().handle_metaserver_communication();
		// Maybe an important message was send on the metaserver,
		// that we should show in game as well.
		std::vector<ChatMessage> msgs;
		InternetGaming::ref().getIngameSystemMessages(msgs);
		for (uint8_t i = 0; i < msgs.size(); ++i)
			send(msgs.at(i));
	}

	// Check if we hear anything from our clients
	while (SDLNet_CheckSockets(d->sockset, 0) > 0) {
		for (size_t i = 0; i < d->clients.size(); ++i) {
			try {
				Client & client = d->clients.at(i);

				while (client.sock && SDLNet_SocketReady(client.sock)) {
					if (!client.deserializer.read(client.sock)) {
						disconnectClient(i, "CONNECTION_LOST", false);
						break;
					}

					//  Handle all available packets immediately after each read, so
					//  that we do not miss any commands (especially DISCONNECT...).
					while (client.sock && client.deserializer.avail()) {
						RecvPacket r(client.deserializer);
						handle_packet(i, r);
					}
				}
			} catch (const DisconnectException & e) {
				disconnectClient(i, e.what());
			} catch (const ProtocolException & e) {
				disconnectClient(i, "PROTOCOL_EXCEPTION", true, boost::lexical_cast<std::string>(e.number()));
			} catch (const std::exception & e) {
				disconnectClient(i, "MALFORMED_COMMANDS", true, e.what());
			}
		}
	}

	// If a pause was forced or if the players all pause, send a ping regularly
        // to keep the sockets up and running
	if ((m_forced_pause || realSpeed() == 0) && (time(nullptr) > (d->lastpauseping + 20))) {
		d->lastpauseping = time(nullptr);

		SendPacket s;
		s.Unsigned8(NETCMD_PING);
		broadcast(s);
	}

	reaper();
}


/**
 * Handle a single received packet.
 *
 * The caller must catch exceptions and disconnect the client as appropriate.
 *
 * \param i the client number
 * \param r the received packet
 */
void NetHost::handle_packet(uint32_t const i, RecvPacket & r)
{
	Client & client = d->clients.at(i);
	uint8_t const cmd = r.Unsigned8();

	if (cmd == NETCMD_DISCONNECT) {
		uint8_t number = r.Unsigned8();
		std::string reason = r.String();
		if (number == 1)
			disconnectClient(i, reason, false);
		else {
			std::string arg = r.String();
			disconnectClient(i, reason, false, arg);
		}
		return;
	}

	if (client.playernum == UserSettings::notConnected()) {
		if (cmd == NETCMD_METASERVER_PING) {
			dedicatedlog("[Host]: Received ping from metaserver.\n");
			// Send PING back
			SendPacket s;
			s.Unsigned8(NETCMD_METASERVER_PING);
			s.send(client.sock);

			// Remove metaserver from list of clients
			client.playernum = UserSettings::notConnected();
			SDLNet_TCP_DelSocket (d->sockset, client.sock);
			SDLNet_TCP_Close (client.sock);
			client.sock = nullptr;
			return;
		}

		// Now we wait for the client to say Hi in the right language,
		// unless the game has already started
		if (d->game)
			throw DisconnectException("GAME_ALREADY_STARTED");

		if (cmd != NETCMD_HELLO)
			throw ProtocolException(cmd);

		uint8_t version = r.Unsigned8();
		if (version != NETWORK_PROTOCOL_VERSION)
			throw DisconnectException("DIFFERENT_PROTOCOL_VERS");

		std::string clientname = r.String();
		client.build_id = r.String();

		welcomeClient(i, clientname);
		return;
	}

	switch (cmd) {
	case NETCMD_PONG:
		dedicatedlog("[Host]: Client %u: got pong\n", i);
		break;

	case NETCMD_SETTING_MAP:
		if (!d->game) {
			// Only valid if the server is dedicated and the client was granted access
			if (!client.dedicated_access)
				throw DisconnectException("NO_ACCESS_TO_SERVER");

			// We want to skip past the name, so read that but don't do anything with it
			r.String();
			std::string path = g_fs->FileSystem::fixCrossFile(r.String());
			bool savegame    = r.Unsigned8() == 1;
			bool scenario    = r.Unsigned8() == 1;
			if (savegame) {
				if (g_fs->FileExists(path)) {
					// Check if file is a saved game and if yes read out the needed data
					try {
						Widelands::Game game;
						Widelands::Game_Preload_Data_Packet gpdp;
						Widelands::Game_Loader gl(path, game);
						gl.preload_game(gpdp);

						// If we are here, it is a saved game file :)
						// Read the needed data from file "elemental" of the used map.
						std::unique_ptr<FileSystem> sg_fs(g_fs->MakeSubFileSystem(path.c_str()));
						Profile prof;
						prof.read("map/elemental", nullptr, *sg_fs);
						Section & s = prof.get_safe_section("global");
						uint8_t nr_players = s.get_safe_int("nr_players");

						d->settings.scenario = false;
						d->hp.setMap(gpdp.get_mapname(), path, nr_players, true);
					} catch (const _wexception &) {}
				}
			} else {
				if (g_fs->FileExists(path)) {
					// Check if file is a map and if yes read out the needed data
					Widelands::Map   map;
					i18n::Textdomain td("maps");
					std::unique_ptr<Widelands::Map_Loader> ml = map.get_correct_loader(path);
					if (ml.get() != nullptr) {
						// Yes it is a map file :)
						map.set_filename(path.c_str());
						ml->preload_map(true);
						d->settings.scenario = scenario;
						d->hp.setMap(map.get_name(), path, map.get_nrplayers(), false);
					}
				}
			}
		}
		break;

	case NETCMD_SETTING_CHANGETRIBE:
		//  Do not be harsh about packets of this type arriving out of order -
		//  the client might just have had bad luck with the timing.
		if (!d->game) {
			uint8_t num = r.Unsigned8();
			if (num != client.playernum)
				// Only valid if the server is dedicated and the client was granted access
				if (!client.dedicated_access)
					throw DisconnectException("NO_ACCESS_TO_PLAYER");
			std::string tribe = r.String();
			bool random_tribe = r.Unsigned8() == 1;
			setPlayerTribe(num, tribe, random_tribe);
		}
		break;

	case NETCMD_SETTING_CHANGESHARED:
		//  Do not be harsh about packets of this type arriving out of order -
		//  the client might just have had bad luck with the timing.
		if (!d->game) {
			uint8_t num = r.Unsigned8();
			if (num != client.playernum)
				// Only valid if the server is dedicated and the client was granted access
				if (!client.dedicated_access)
					throw DisconnectException("NO_ACCESS_TO_PLAYER");
			setPlayerShared(num, r.Unsigned8());
		}
		break;

	case NETCMD_SETTING_CHANGETEAM:
		if (!d->game) {
			uint8_t num = r.Unsigned8();
			if (num != client.playernum)
				// Only valid if the server is dedicated and the client was granted access
				if (!client.dedicated_access)
					throw DisconnectException("NO_ACCESS_TO_PLAYER");
			setPlayerTeam(num, r.Unsigned8());
		}
		break;

	case NETCMD_SETTING_CHANGEINIT:
		if (!d->game) {
			uint8_t num = r.Unsigned8();
			if (num != client.playernum)
				// Only valid if the server is dedicated and the client was granted access
				if (!client.dedicated_access)
					throw DisconnectException("NO_ACCESS_TO_PLAYER");
			d->npsb.toggle_init(num);
		}
		break;

	case NETCMD_SETTING_CHANGEPOSITION:
		if (!d->game) {
			uint8_t const pos = r.Unsigned8();
			switchToPlayer(client.usernum, pos);
		}
		break;

	case NETCMD_SETTING_PLAYER:
		if (!d->game) {
			// Only valid if the server is dedicated and the client was granted access
			if (!client.dedicated_access)
				throw DisconnectException("NO_ACCESS_TO_SERVER");
			d->hp.nextPlayerState(r.Unsigned8());
		}
		break;

	case NETCMD_WIN_CONDITION:
		if (!d->game) {
			// Only valid if the server is dedicated and the client was granted access
			if (!client.dedicated_access)
				throw DisconnectException("NO_ACCESS_TO_SERVER");
			d->hp.nextWinCondition();
		}
		break;

	case NETCMD_LAUNCH:
		if (!d->game) {
			// Only valid if the server is dedicated and the client was granted access
			if (!client.dedicated_access)
				throw DisconnectException("NO_ACCESS_TO_SERVER");
			if (!canLaunch())
				throw DisconnectException("START_SENT_NOT_READY");
			d->dedicated_start = true;
		}
		break;

	case NETCMD_TIME:
		if (!d->game)
			throw DisconnectException("TIME_SENT_NOT_READY");
		recvClientTime(i, r.Signed32());
		break;

	case NETCMD_PLAYERCOMMAND: {
		if (!d->game)
			throw DisconnectException("PLAYERCMD_WO_GAME");
		int32_t time = r.Signed32();
		Widelands::PlayerCommand & plcmd = *Widelands::PlayerCommand::deserialize(r);
		log
			("[Host]: Client %u (%u) sent player command %i for %i, time = %i\n",
			 i, client.playernum, plcmd.id(), plcmd.sender(), time);
		recvClientTime(i, time);
		if (plcmd.sender() != client.playernum + 1)
			throw DisconnectException("PLAYERCMD_FOR_OTHER");
		sendPlayerCommand(plcmd);
	} break;

	case NETCMD_SYNCREPORT: {
		if (!d->game || !d->syncreport_pending || client.syncreport_arrived)
			throw DisconnectException("UNEXPECTED_SYNC_REP");
		int32_t time = r.Signed32();
		r.Data(client.syncreport.data, 16);
		client.syncreport_arrived = true;
		recvClientTime(i, time);
		checkSyncReports();
		break;
	}

	case NETCMD_CHAT: {
		ChatMessage c;
		c.time = time(nullptr);
		c.playern = d->settings.users.at(client.usernum).position;
		c.sender = d->settings.users.at(client.usernum).name;
		c.msg = r.String();
		if (c.msg.size() && *c.msg.begin() == '@') {
			// Personal message
			std::string::size_type const space = c.msg.find(' ');
			if (space >= c.msg.size() - 1)
				break;
			c.recipient = c.msg.substr(1, space - 1);
			c.msg = c.msg.substr(space + 1);
		}
		send(c);
		break;
	}

	case NETCMD_SETSPEED: {
		client.desiredspeed = r.Unsigned16();
		updateNetworkSpeed();
		break;
	}

	case NETCMD_NEW_FILE_AVAILABLE: {
		if (!file) // Do we have a file for sending
			throw DisconnectException("REQUEST_OF_N_E_FILE");
		sendSystemMessageCode
			("STARTED_SENDING_FILE", file->filename, d->settings.users.at(client.usernum).name);
		sendFilePart(client.sock, 0);
		// Remember client as "currently receiving file"
		d->settings.users[client.usernum].ready = false;
		SendPacket s;
		s.Unsigned8(NETCMD_SETTING_USER);
		s.Unsigned32(client.usernum);
		writeSettingUser(s, client.usernum);
		broadcast(s);
		break;
	}

	case NETCMD_FILE_PART: {
		if (!file) // Do we have a file for sending
			throw DisconnectException("REQUEST_OF_N_E_FILE");
		uint32_t part = r.Unsigned32();
		std::string x = r.String();
		if (x != file->md5sum) {
			dedicatedlog("[Host]: File transfer checksum missmatch %s != %s\n", x.c_str(), file->md5sum.c_str());
			return; // Surely the file was changed, so we cancel here.
		}
		if (part >= file->parts.size())
			throw DisconnectException("REQUEST_OF_N_E_FILEPART");
		if (part == file->parts.size() - 1) {
			sendSystemMessageCode
				("COMPLETED_FILE_TRANSFER", file->filename, d->settings.users.at(client.usernum).name);
			d->settings.users[client.usernum].ready = true;
			SendPacket s;
			s.Unsigned8(NETCMD_SETTING_USER);
			s.Unsigned32(client.usernum);
			writeSettingUser(s, client.usernum);
			broadcast(s);
			return;
		}
		++part;
		if (part % 100 == 0)
			sendSystemMessageCode
				("SENDING_FILE_PART",
				 (boost::format("%i/%i") % part % (file->parts.size() + 1)).str(),
				 file->filename, d->settings.users.at(client.usernum).name);
		sendFilePart(client.sock, part);
		break;
	}

	default:
		throw ProtocolException(cmd);
	}
}

void NetHost::sendFilePart(TCPsocket csock, uint32_t part) {
	assert(part < file->parts.size());

	uint32_t left = file->bytes - NETFILEPARTSIZE * part;
	uint32_t size = (left > NETFILEPARTSIZE) ? NETFILEPARTSIZE : left;

	// Send the part
	SendPacket s;
	s.Unsigned8(NETCMD_FILE_PART);
	s.Unsigned32(part);
	s.Unsigned32(size);
	s.Data(file->parts[part].part, size);
	s.send(csock);
}


void NetHost::disconnectPlayerController(uint8_t const number, const std::string & name)
{
	dedicatedlog("[Host]: disconnectPlayerController(%u, %s)\n", number, name.c_str());

	for (uint32_t i = 0; i < d->settings.users.size(); ++i) {
		if (d->settings.users.at(i).position == number) {
			if (!d->game) {
				// Remove player name
				PlayerSettings & p = d->settings.players.at(number);
				std::string temp(" ");
				temp += name;
				temp += " ";
				std::string temp2(p.name);
				temp2 = temp2.erase(p.name.find(temp), temp.size());
				setPlayerName(number, temp2);
			}
			return;
		}
	}

	setPlayerState(number, PlayerSettings::stateOpen);
	if (d->game)
		initComputerPlayer(number + 1);
}

void NetHost::disconnectClient
	(uint32_t const number, const std::string & reason, bool const sendreason, const std::string & arg)
{
	assert(number < d->clients.size());

	Client & client = d->clients.at(number);

	// If the client was completely connected before the disconnect, free the
	// user settings and send changes to the clients
	if (client.usernum >= 0) {
		uint8_t position = d->settings.users.at(client.usernum).position;
		d->settings.users.at(client.usernum).position = UserSettings::notConnected();
		client.playernum = UserSettings::notConnected();
		if (position <= UserSettings::highestPlayernum()) {
			disconnectPlayerController(position, d->settings.users.at(client.usernum).name);
		}
		// Do NOT reset the clients name in the corresponding UserSettings, that way we keep the name for the
		// statistics.
		// d->settings.users.at(client.usernum).name = std::string();

		// Broadcast the user changes to everybody
		sendSystemMessageCode("CLIENT_X_LEFT_GAME", d->settings.users.at(client.usernum).name, reason, arg);

		SendPacket s;
		s.Unsigned8(NETCMD_SETTING_USER);
		s.Unsigned32(client.usernum);
		writeSettingUser(s, client.usernum);
		broadcast(s);

		// Just for statistics
		DedicatedLog::get()->client_logout();
	} else
		sendSystemMessageCode("UNKNOWN_LEFT_GAME", reason, arg);

	dedicatedlog("[Host]: disconnectClient(%u, %s, %s)\n", number, reason.c_str(), arg.c_str());

	if (client.sock) {
		if (sendreason) {
			SendPacket s;
			s.Unsigned8(NETCMD_DISCONNECT);
			s.Unsigned8(arg.empty() ? 1 : 2);
			s.String(reason);
			if (!arg.empty())
				s.String(arg);
			s.send(client.sock);
		}

		SDLNet_TCP_DelSocket (d->sockset, client.sock);
		SDLNet_TCP_Close (client.sock);
		client.sock = nullptr;
	}

	if (d->game) {
		checkHungClients();
		if (m_is_dedicated) {
			// Check whether there is at least one client connected. If not, stop the game.
			for (uint32_t i = 0; i < d->clients.size(); ++i)
				if (d->clients.at(i).playernum != UserSettings::notConnected())
					return;
			d->game->end_dedicated_game();
			dedicatedlog("[Dedicated] Stopping the running game...\n");
		}
	}

}

/**
 * The grim reaper. This finally erases disconnected clients from the clients
 * array.
 *
 * Calls this when you're certain that nobody is holding any client indices or
 * iterators, since this function will invalidate them.
 */
void NetHost::reaper()
{
	uint32_t index = 0;
	while (index < d->clients.size())
		if (d->clients.at(index).sock)
			++index;
		else
			d->clients.erase(d->clients.begin() + index);
}


void NetHost::report_result
	(uint8_t p_nr, Widelands::PlayerEndResult result, const std::string & info)
{
	// Send to game
	Widelands::PlayerEndStatus pes;
	Widelands::Player* player = d->game->get_player(p_nr);
	assert(player);
	pes.player = player->player_number();
	pes.time = d->game->get_gametime();
	pes.result = result;
	pes.info = info;
	d->game->player_manager()->add_player_end_status(pes);

	// there might be more than one client that control this Widelands player
	// and maybe even none -> computer player
	for (uint16_t i = 0; i < d->settings.users.size(); ++i) {
		UserSettings & user = d->settings.users.at(i);
		if (user.position == p_nr - 1) {
			user.result               = result;
			user.win_condition_string = info;
			if (result == Widelands::PlayerEndResult::PLAYER_LOST) {
				sendSystemMessageCode("PLAYER_DEFEATED", user.name);
			}
		}
	}

	dedicatedlog
		("NetHost::report_result(%d, %u, %s)\n",
		 player->player_number(), static_cast<uint8_t>(result), info.c_str());
}
