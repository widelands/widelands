/*
 * Copyright (C) 2002 by Holger Rapp
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

// 2002-02-10	sft+	made setup_searchpaths work for win32
// 2002-02-11	sft+	made setup_searchpaths work PROPERLY for win32

#include "fileloc.h"
#include "setup.h"
#include "graphic.h"
#include "font.h"
#include "ui.h"
#include "cursor.h"
#include "output.h"

#include <string.h>

#ifndef WIN32
#include <sys/stat.h>
#include <sys/types.h>
#endif

/** void setup_fonthandler(void)
 *
 * Sets up the global font handler for the needs in widelands
 *
 * Args: None
 * Returns: Nothing
 */
void setup_fonthandler(void) {
		  // Setting Font_Handler up
		  const char* buf=g_fileloc.locate_file("fixed_font1.wff", TYPE_FONT);
		  if(!buf) {
					 tell_user("fixed_font1.wwf:  File not found. Check your installation.");
					 exit(0);
		  }
		  if(g_fh.load_font( buf, FIXED_FONT1 ) != RET_OK) {
					 char mbuf[200];
					 sprintf(mbuf, "%s: Invalid File, load error, or wrong version. Check for new versions.", buf);
					 tell_user(mbuf);
					 exit(0);
		  }
}

/** void setup_ui(void)
 *
 * Sets up the user interface for the needs in widelands
 *
 * Args: none
 * Returns: Nothing
 */
void setup_ui(void) {
		  // Init cursor
		  Pic *cursor=new Pic;
		  const char* buf=g_fileloc.locate_file("cursor.bmp", TYPE_PIC);
		  if(!buf) {
					 tell_user("cursor.bmp:  File not found. Check your installation.");
					 exit(0);
		  }

		  cursor->load(buf);
		  cursor->set_clrkey(0,0,255);
		  g_cur.set_pic(cursor);

		  // Button class
		  Button::setup_ui();

		  // list select class
		  Listselect::setup_ui();
}

/** void setup_searchpaths(void)
 *
 * Sets the filelocators default searchpaths, this is OS specific
 *
 * Args: None
 * Returns: Nothing
 */
void setup_searchpaths(void)
{
	static File_Locator fileloc;

	char* buf;
	char cmd[MAX_PATHL];
#ifndef	WIN32
	buf=getenv("HOME");
	strcpy(cmd, buf);
	strcat(cmd, "/.widelands");
	mkdir(cmd, 0x1FF);
	g_fileloc.add_searchdir(cmd, MAX_DIRS-1);
	g_fileloc.set_def_writedir(MAX_DIRS-1);
	g_fileloc.add_searchdir(PKGDATADIR, MAX_DIRS-2);

#else
	uint i=0, n=0;
	buf = GetCommandLine();
	while (buf[i]==' ' || buf[i]=='\"' )
		i++;
	while (buf[i]!='\"' && i<strlen(buf))
	{
		cmd[n] = buf[i];
		i++;
		n++;
	}
	cmd[n] = 0;
	for (int j=strlen(cmd)-1; j>=0; j--)
		if (cmd[j] == '\\')
		{
			cmd[j] = 0;
			break;
		}
	g_fileloc.add_searchdir(cmd, MAX_DIRS-2);
#endif
}
