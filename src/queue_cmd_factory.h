/*
 * Copyright (C) 2002-2004, 2008-2009 by the Widelands Development Team
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

#ifndef QUEUE_CMD_FACTORY_H
#define QUEUE_CMD_FACTORY_H

#include <stdint.h>

namespace Widelands {

class GameLogicCommand;

/*
 * This creates the correct queue commands
 * from the queue command file ids
 */
struct Queue_Cmd_Factory {
	static GameLogicCommand & create_correct_queue_command(uint32_t id);
};

};

#endif
