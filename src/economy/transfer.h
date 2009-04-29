/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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

#ifndef S__TRANSFER_H
#define S__TRANSFER_H

#include "route.h"

namespace Widelands {
struct Game;
struct PlayerImmovable;
struct Request;
struct WareInstance;
struct Worker;

/**
 * Whenever an item or worker is transferred to fulfill a Request,
 * a Transfer is allocated to describe this transfer.
 *
 * Transfers are always created and destroyed by a Request instance.
 *
 * Call get_next_step() to find out where you should go next. If
 * get_next_step() returns 0, the transfer is complete or cannot be
 * completed. Call finish() if success is true, fail() otherwise.
 * Call fail() if something really bad has happened (e.g. the worker
 * or ware was destroyed).
 *
 * \todo The mentioned function fail() does not exist!
 */
struct Transfer {
	friend struct Request;

	Transfer(Game &, Request &, WareInstance &);
	Transfer(Game &, Request &, Worker       &);
	~Transfer();

	Request & request() const {return m_request;}
	bool is_idle() const {return m_idle;}

	void set_idle(bool idle);

public:
	/// Called by the controlled ware or worker
	PlayerImmovable * get_next_step(PlayerImmovable *, bool & psuccess);
	void has_finished();
	void has_failed();

private:
	void tlog(char const * fmt, ...) PRINTF_FORMAT(2, 3);

	Game         & m_game;
	Request      & m_request;
	WareInstance * m_item;    ///< non-null if ware is an item
	Worker       * m_worker;  ///< non-null if ware is a worker
	Route        m_route;

	bool m_idle; ///< an idle transfer can be fail()ed if the item feels like it
};

};

#endif


