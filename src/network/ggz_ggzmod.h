/*
* Copyright (C) 2004-2010 by the Widelands Development Team
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

#ifndef GGZ_GGZMOD_H
#define GGZ_GGZMOD_H

#include <ggzmod.h>
#include <stdint.h>
#include <string>

#ifdef USE_BOOST_THREADS
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#endif

/**
 * Does the ggzmod part of ggz. Ggzmod is the part of ggz client which
 * implements the actual game part of ggz.
 * @note ggzmod comunicates with the ggzcore object through a socket. This
 *   may cause deadlocks. For example 
 */
class ggz_ggzmod
{
public:
	ggz_ggzmod();
	~ggz_ggzmod();

	/*  only used by main main thread to initialize and eventually create the
	 * thread*/
	bool connect();

	/*thread safe? */
	bool connected() { return m_connected; }
	void disconnect(bool err = false);
#ifdef USE_BOOST_THREADS
	/// Process does nothing if threads are used.
	void process() {}
	bool data_pending() { return false; }
#else
	void process();
	bool data_pending();
#endif

	int32_t datafd();
	void set_spectator();
	void set_player();

	/**
	 * Returns the ggz nick name of the player. 
	 */
	std::string playername();

	inline void set_fds (fd_set & set) {
#ifndef USE_BOOST_THREADS
		if (m_server_fd >= 0 and m_server_fd < FD_SETSIZE)
			FD_SET(m_server_fd, &set);
#else
		boost::shared_lock<boost::shared_mutex> sl(threadlock);
#endif
		if (m_data_fd >= 0 and m_data_fd < FD_SETSIZE)
			FD_SET(m_data_fd, &set);
	}

private:

#ifdef USE_BOOST_THREADS
	/// main function of the worker thread. If USE_BOOST_THREADS is defined this
	/// connect() created a thread which runs this function.
	void _thread_main();

	boost::thread ggzmodthread;

	/// This mutex is used to lock access to members of this class. Some members
	/// of this class are used by main thread and the ggzmod worker thread. A
	/// lock must be aquired before using one of them.
	boost::shared_mutex threadlock;

	//bool m_disconnect;
	/// If set to true thread main loop exit as soon as possible.
	bool m_requested_exit;
#endif
	ggz_ggzmod(const ggz_ggzmod &) {}

	/// this is called by _thread_main() or process() to call seat request
	/// functions. This is called if m_changed is set.
	void changeSeat();

	/// Callback for events from libggzmod. This only calls ggzmod_server() wiht
	/// object.
	/// @note In threadded mode this is only called from worker thread by
	///       @ref ggzmod_dispatch
	static void eventServer(GGZMod *, GGZModEvent, void const * cbdata);

	/// This get only called from eventServer()
	void ggzmod_server(GGZMod *, GGZModEvent, void const * cbdata);

	/// filedescriptor to communicate with the widelands_server module of ggzd.
	/// @note only used by worker thread.
	int32_t m_data_fd;

	/// fd to communicate with the ggzcore object of this process. With that we
	/// talk to ourself. Not a good idea.
	/// Read by main thread and written once by worker thread. Shoul be possible
	/// to read this from main thread without locking.
	int32_t m_server_fd;

	/// only used by main thread. Worker thread can assume connection as long
	/// as @ref m_request_exited and @ref m_error_exit are not set.
	bool m_connected;

	/// GGZMod libary handle
	/// @note only use by worker thread
	GGZMod * m_mod;

	/// Somethin was changed. So additional processing neccesarry. Currently only
	/// used if m_seat_desired is set.
	/// @note is used by both treads. Needs to be locked.
	bool m_changed;

	/// Do we have a seat or are we just a spectator?
	/// @note only used from worker thread
	bool m_seat;

	/// Do we want a seat or are we just spectator
	/// @note is used by both treads. Needs to be locked.
	bool m_seat_desired;

	/// we have requested a seat change and wait for answer. Only changed by
	/// a ggzmod event and @ref changeSeat()
	/// @note only used from worker thread
	bool m_pending_seat_change;

	/// indicates a emergency exit. Most probably ggzcore had an error. We must
	/// not use any processing functions anymore.
	/// @note is used by both treads. Needs to be locked.
	bool m_error_exit;
};
#endif // GGZ_GGZMOD_H
