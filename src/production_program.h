/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__PRODUCTION_PROGRAM_H
#define __S__PRODUCTION_PROGRAM_H

#include "log.h"

#include <cassert>
#include <string>
#include <vector>

#include <stdint.h>

struct EncodeData;
struct ProductionSite_Descr;
class Profile;

struct ProductionAction {
	enum Type {
		actSleep,   // iparam1 = sleep time in milliseconds
		actWorker,  // sparam1 = worker program to run
		actConsume, // sparam1 = consume this ware, has to be an input, iparam1 number to consume
		actAnimate, // sparam1 = activate this animation until timeout
		actProduce, // sparam1 = ware to produce. the worker carries it outside
		actCheck,   // sparam1 = check if the given input ware is available, iparam1 number to check for
		actMine,    // sparam1 = resource, iparam1=how far to mine, iparam2=up to max mine, iparam3=chance below
		actCall,    // sparam1 = name of sub-program
		actSet,     // iparam1 = flags to set, iparam2 = flags to unset
		actPlayFX,  // sparam1 = sound_fx_name to play

		// This is ONLY for Training Porpouses!
      actCheckSoldier, // sparam = attribute asking to, iparam1 = level requested
      actTrain,   // sparam = attribute asking to, iparam1 = level requested, iparam2 = level to upgrade
	};

	enum {
		// When pfCatch is set, failures of the current program cause the
		// termination of this program, but the parent program will continue
		// to run.
		// When pfCatch is not set, the parent program will fail as well.
		pfCatch = (1 << 0),

		// Ending this program has no effect on productivity statistics.
		// However, child programs can still affect statistics
		pfNoStats = (1 << 1),
	};

	Type        type;
	int32_t         iparam1;
	int32_t         iparam2;
	int32_t         iparam3;
	std::string sparam1;
};


/*
class ProductionProgram
-----------------------
Holds a series of actions to perform for production.
*/
struct ProductionProgram {
	ProductionProgram(const std::string & name);

	std::string get_name() const {return m_name;}
	int32_t get_size() const {return m_actions.size();}
	const ProductionAction* get_action(int32_t idx) const {
		assert(idx >= 0);
		assert(static_cast<uint32_t>(idx) < m_actions.size());
		return &m_actions[idx];
	}

	const std::vector<ProductionAction>& get_all_actions() {return m_actions;};

	void parse(std::string directory, Profile* prof, std::string name,
		ProductionSite_Descr* building, const EncodeData* encdata);

private:
	std::string                   m_name;
	std::vector<ProductionAction> m_actions;
};




#endif
