/*
 * Copyright (C) 2002-2004, 2008 by the Widelands Development Team
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

#ifndef QUEUE_CMD_IDS_H
#define QUEUE_CMD_IDS_H

/*
 * This file contains the ids for the different
 * cmd_queue cmds. They are needed to save those
 * files to disk and (even more important) to load
 * them again.
 *
 * Some of this Commands (the player commands) have another
 * index, but this one is completly independant and only for
 * writing to files
 */

/* ID zero is reserved and must never be used */
#define QUEUE_CMD_NONE                   0

/* PLAYER COMMANDS BELOW */
#define QUEUE_CMD_BUILD                  1
#define QUEUE_CMD_FLAG                   2
#define QUEUE_CMD_BUILDROAD              3
#define QUEUE_CMD_FLAGACTION             4
#define QUEUE_CMD_STOPBUILDING           5
#define QUEUE_CMD_ENHANCEBUILDING        6
#define QUEUE_CMD_BULLDOZE               7
#define QUEUE_CMD_CHANGETRAININGOPTIONS  8
#define QUEUE_CMD_DROPSOLDIER            9
#define QUEUE_CMD_CHANGESOLDIERCAPACITY 10
#define QUEUE_CMD_ENEMYFLAGACTION       11

#define QUEUE_CMD_DESTROY_MAPOBJECT    127
#define QUEUE_CMD_ACT                  128
#define QUEUE_CMD_CHECK_EVENTCHAIN     129
#define QUEUE_CMD_INCORPORATE          130

#define QUEUE_CMD_CALL_ECONOMY_BALANCE 200

#define QUEUE_CMD_NETCHECKSYNC         250
#define QUEUE_CMD_REPLAYSYNCWRITE      251
#define QUEUE_CMD_REPLAYSYNCREAD       252

#endif
