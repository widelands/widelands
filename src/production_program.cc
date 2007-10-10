/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "production_program.h"

#include "helper.h"
#include "productionsite.h"
#include "profile.h"
#include "tribe.h"
#include "worker_program.h"

#include <stdio.h>


ProductionProgram::ProductionProgram(const std::string & name) : m_name(name) {}


/*
===============
ProductionProgram::parse

Parse a program. The building is parsed completly. hopefully
===============
*/
void ProductionProgram::parse(std::string directory, Profile* prof,
	std::string name, ProductionSite_Descr* building, const EncodeData* encdata)
{
	Section* sprogram = prof->get_safe_section(name.c_str());

	for (uint32_t idx = 0; ; ++idx) {
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%i", idx);
		const char * const string = sprogram->get_string(buffer, 0);
		if (!string)
			break;

		const std::vector<std::string> cmd(split_string(string, " \t\r\n"));
		if (!cmd.size())
			continue;

		ProductionAction act;

		if (cmd[0] == "sleep") {
			char* endp;

			if (cmd.size() != 2)
				throw wexception("Line %i: Usage: sleep <time in ms>", idx);

			act.type = ProductionAction::actSleep;
			act.iparam1 = strtol(cmd[1].c_str(), &endp, 0);

			if (endp && *endp)
            throw wexception("Line %i: bad integer '%s'", idx, cmd[1].c_str());
		} else if (cmd[0] == "consume") {
         if (cmd.size() != 2 && cmd.size() != 3)
            throw wexception("Line %i: Usage: consume <ware>[,<ware>,<ware>..] [number] (no blanks between wares)", idx);

			{
				Section * const section = prof->get_safe_section("inputs");
				const std::vector<std::string> wares(split_string(cmd[1], ","));
				const std::vector<std::string>::const_iterator wares_end =
					wares.end();
				for
					(std::vector<std::string>::const_iterator it = wares.begin();
					 it != wares_end;
					 ++it)
					if (not section->get_string(it->c_str(), 0))
						throw wexception
							("Line %i: Ware %s is not in [inputs]",
							 idx, cmd[1].c_str());
			}

         act.type = ProductionAction::actConsume;
         act.sparam1 = cmd[1];
         int32_t how_many=1;
			if (cmd.size() == 3) {
            char* endp;
            how_many = strtol(cmd[2].c_str(), &endp, 0);
            if (endp && *endp)
               throw wexception("Line %i: bad integer '%s'", idx, cmd[1].c_str());

			}
         act.iparam1 = how_many;
		}  else if (cmd[0] == "check") {
			if (cmd.size() != 2 && cmd.size() != 3)
				throw wexception("Line %i: Usage: checking <ware>[,<ware>,<ware>..] [number] (no blanks between wares)", idx);

			{
				Section * const section = prof->get_safe_section("inputs");
				const std::vector<std::string> wares(split_string(cmd[1], ","));
				const std::vector<std::string>::const_iterator wares_end =
					wares.end();
				for
					(std::vector<std::string>::const_iterator it = wares.begin();
					 it != wares_end;
					 ++it)
					if (not section->get_string(it->c_str(), 0))
						throw wexception
							("Line %i: Ware %s is not in [inputs]",
							 idx, cmd[1].c_str());
			}
			act.type = ProductionAction::actCheck;
			act.sparam1 = cmd[1];
         int32_t how_many=1;
			if (cmd.size() == 3) {
            char* endp;
            how_many = strtol(cmd[2].c_str(), &endp, 0);
            if (endp && *endp)
               throw wexception("Line %i: bad integer '%s'", idx, cmd[1].c_str());

			}
         act.iparam1 = how_many;

		} else if (cmd[0] == "produce") {
			if (cmd.size() != 2)
				throw wexception("Line %i: Usage: produce <ware>", idx);

			if (!building->is_output(cmd[1]))
				throw wexception("Line %i: Ware %s is not in [outputs]\n", idx,
					cmd[1].c_str());

			act.type = ProductionAction::actProduce;
			act.sparam1 = cmd[1];
		} else if (cmd[0] == "worker") {
			if (cmd.size() != 2)
				throw wexception("Line %i: Usage: worker <program name>", idx);

			act.type = ProductionAction::actWorker;
			act.sparam1 = cmd[1];

			//  Quote form "void ProductionSite::program_act(Game*)":
			//  "Always main worker is doing stuff"
			const char * const main_worker_name
				= (*building->get_workers())[0].name.c_str();
			const Tribe_Descr & tribe = building->tribe();
			const Workarea_Info & worker_workarea_info
				= tribe.get_worker_descr(tribe.get_safe_worker_index(main_worker_name))
				->get_program(act.sparam1.c_str())->get_workarea_info();
			const Workarea_Info::const_iterator worker_workarea_info_end =
				worker_workarea_info.end();
			Workarea_Info & building_workarea_info = building->m_workarea_info;
			for
				(Workarea_Info::const_iterator it = worker_workarea_info.begin();
				 it != worker_workarea_info_end;
				 ++it)
			{
				std::set<std::string> & building_radius_infos =
					building_workarea_info[it->first];
				const std::set<std::string> & descriptions = it->second;
				const std::set<std::string>::const_iterator descriptions_end =
					descriptions.end();
				for
					(std::set<std::string>::const_iterator de = descriptions.begin();
					 de != descriptions_end;
					 ++de)
				{
					std::string description = building->descname();
					description += ' ';
					description += m_name;
					description += " worker ";
					description += main_worker_name;
					description += *de;
					building_radius_infos.insert(description);
				}
			}
		} else if (cmd[0] == "animation") {
			char* endp;

			if (cmd.size() != 3)
				throw wexception("Usage: animation <name> <time>");

			act.type = ProductionAction::actAnimate;

			// dynamically allocate animations here
			if (not building->is_animation_known(cmd[1].c_str())) {
            Section* s = prof->get_safe_section(cmd[1].c_str());
            act.iparam1 = g_anim.get(directory.c_str(), s, 0, encdata);
            building->add_animation(cmd[1].c_str(), act.iparam1);
			} else
            act.iparam1 = building->get_animation(cmd[1].c_str());

			if (cmd[1] == "idle")
				/* XXX */
				throw wexception("Idle animation is default, no calling senseful!");

			act.iparam2 = strtol(cmd[2].c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Bad duration '%s'", cmd[2].c_str());

			if (act.iparam2 <= 0)
				throw wexception("animation duration must be positive");
		} else if (cmd[0] == "mine") {
         char* endp;

			if (cmd.size() != 5)
				throw wexception("Usage: mine <resource> <area> <up to %%> <chance after %%>");

			act.type = ProductionAction::actMine;
			act.sparam1=cmd[1]; // what to mine
         act.iparam1=strtol(cmd[2].c_str(), &endp, 0);
         if (endp && *endp)
            throw wexception("Bad area '%s'", cmd[2].c_str());
         act.iparam2=strtol(cmd[3].c_str(), &endp, 0);
         if (endp && *endp || act.iparam2>100)
            throw wexception("Bad maximum amount: '%s'", cmd[3].c_str());
         act.iparam3=strtol(cmd[4].c_str(), &endp, 0);
         if (endp && *endp || act.iparam3>100)
            throw wexception("Bad chance after maximum amount is empty: '%s'", cmd[4].c_str());

				 std::string description = building->descname();
				 description += ' ';
				 description += name;
				 description += " mine ";
				 description += act.sparam1;
				 building->m_workarea_info[act.iparam1].insert(description);
		} else if (cmd[0] == "call") {
			if (cmd.size() != 2)
				throw wexception("Usage: call <program>");

			act.type = ProductionAction::actCall;

			act.sparam1 = cmd[1];
		} else if (cmd[0] == "set") {
			if (cmd.size() < 2)
				throw wexception("Usage: set <+/-flag>...");

			act.type = ProductionAction::actSet;

			act.iparam1 = act.iparam2 = 0;

			for (uint32_t i = 1; i < cmd.size(); ++i) {
				std::string cmdname;
				int32_t flag;
				char c = cmd[i][0];

				cmdname = cmd[i].substr(1);
				if (cmdname == "catch")
					flag = ProductionAction::pfCatch;
				else if (cmdname == "nostats")
					flag = ProductionAction::pfNoStats;
				else
					throw wexception("Unknown flag cmdname '%s'", cmdname.c_str());

				if (c == '+')
					act.iparam1 |= flag;
				else if (c == '-')
					act.iparam2 |= flag;
				else
					throw wexception("+/- expected in front of flag (%s)", cmd[i].c_str());
			}

			if (act.iparam1 & act.iparam2)
				throw wexception("Ambiguous set command");
		} else if (cmd[0] == "check_soldier") {
			if (cmd.size() != 3)
				throw wexception("Usage: check_soldier <attribute> <level>");

			act.type = ProductionAction::actCheckSoldier;

			act.iparam1 = act.iparam2 = 0;

			if
				(cmd[1] == "hp"
				 or
				 cmd[1] == "attack"
				 or
				 cmd[1] == "defense"
				 or
				 cmd[1] == "evade")
				act.sparam1 = cmd[1];
			else
				throw wexception
					("check_soldier needs 'hp', 'attack', 'defense' or 'evade' "
					 "parameter");

			int32_t how_many = 1;
			char * endp;
			how_many = strtol(cmd[2].c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Line %i: bad integer '%s'", idx, cmd[1].c_str());

			act.iparam1 = how_many;

			if (act.iparam1 < 0)
				throw wexception("Level must be greater than 0");
		} else if (cmd[0] == "train") {
			if (cmd.size() != 4)
				throw wexception("Usage: train <attribute> <current_level> <new_level>");

			act.type = ProductionAction::actTrain;

			act.iparam1 = act.iparam2 = 0;

			if
				(cmd[1] == "hp"
				 or
				 cmd[1] == "attack"
				 or
				 cmd[1] == "defense"
				 or
				 cmd[1] == "evade")
				act.sparam1 = cmd[1];
			else
				throw wexception
					("train needs 'hp', 'attack', 'defense' or 'evade' parameter");

			int32_t how_many=1;
			char* endp;
			how_many = strtol(cmd[2].c_str(), &endp, 0);
			if (endp && *endp)
			  throw wexception("Line %i: bad integer '%s'", idx, cmd[1].c_str());

			act.iparam1 = how_many;

			how_many = strtol(cmd[3].c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Line %i: bad integer '%s'", idx, cmd[1].c_str());

			act.iparam2 = how_many;

			if (act.iparam1 >= act.iparam2)
				throw wexception("current_level must be lesser than new_level");
		} else if (cmd[0] == "playFX") {
			if (cmd.size()<2 || cmd.size()>3)
				throw wexception("Usage: play <sound_fx_name> [priority]");

			act.type = ProductionAction::actPlayFX;

			act.sparam1=cmd[1];
			//TODO: check if fx exists, load fx, lots of other checks for aprameter correctness
			if (cmd.size()==2)
				act.iparam1=127;
			else
				act.iparam1=atoi(cmd[2].c_str());


		} else
			throw wexception("Line %i: unknown command '%s'", idx, cmd[0].c_str());

		m_actions.push_back(act);
	}

	// Check for numbering problems
	if (sprogram->get_num_values() != m_actions.size())
		throw wexception("Line numbers appear to be wrong");
}
