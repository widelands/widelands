/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include <SDL.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <vector>
#include "error.h"
#include "filesystem.h"
#include "machdep.h"
#include "profile.h"
#include "system.h"
#include "wexception.h"
#include "font_handler.h"
#include "sound_handler.h"
#include "wlapplication.h"

#include "constants.h"
#include "network_ggz.h"

#ifdef DEBUG
#ifndef __WIN32__
#include <signal.h>

static int pid_me=0, pid_peer=0;

static volatile int may_run=0;

static void signal_handler (int sig)
{
	may_run++;
}

static void quit_handler ()
{
	kill (pid_peer, SIGTERM);
	sleep (2);
	kill (pid_peer, SIGKILL);
}

void yield_double_game ()
{
	if (pid_me==0)
		return;

	if (may_run>0) {
		may_run--;
		kill (pid_peer, SIGUSR1);
	}

	if (may_run==0)
		usleep (500000);

	// using sleep instead of pause avoids a race condition
	// and a deadlock during connect
}

void init_double_game ()
{
	if (pid_me!=0)
		return;

	pid_me=getpid();
	pid_peer=fork();

	assert (pid_peer>=0);

	if (pid_peer==0) {
		pid_peer=pid_me;
		pid_me=getpid();

		may_run=1;
	}

	signal (SIGUSR1, signal_handler);

	atexit (quit_handler);
}
#endif // WIN32
#endif // DEBUG

