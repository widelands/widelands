/*
 * Copyright (C) 2001 by Holger Rapp 
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

#include "fileloc.h"
#include "setup.h"
#include "graphic.h"
#include "font.h"
#include "ui.h"

#include <string.h>

/** void setup_fonthandler(void) 
 *
 * Sets up the global font handler for the needs in widelands
 *
 * Args: None
 * Returns: Nothing
 */
void setup_fonthandler(void) {
		  // Setting Font_Handler up
		  Pic *bmp = new Pic;
		  const char* buf=g_fileloc.locate_file("fixed_font1.bmp", TYPE_PIC);
		  assert(buf);
		  bmp->load(buf);
		  bmp->set_clrkey(DEF_CLRKEY);
		  g_fh.set_font(0, bmp, FONT_W, FONT_H);
		  
		  buf=g_fileloc.locate_file("fixed_font2.bmp", TYPE_PIC);
		  assert(buf);
		  bmp = new Pic;
		  bmp->load(buf);
		  bmp->set_clrkey(DEF_CLRKEY);
		  g_fh.set_font(1, bmp, FONT_W, FONT_H);
}

/** void setup_ui(void) 
 *
 * Sets up the user interface for the needs in widelands
 *
 * Args: none
 * Returns: Nothing
 */
void setup_ui(void) {
		  
		  // Init the window
		  // Window's without clrkey, so it's drawn WAY faster
		  Pic bmp;
		  const char *buf=g_fileloc.locate_file("win_l_border.bmp", TYPE_PIC);
		  assert(buf);
		  bmp.load(buf);
//		  bmp.set_clrkey(DEF_CLRKEY);
		  Window::set_l_border(&bmp);
		  
		  buf=g_fileloc.locate_file("win_r_border.bmp", TYPE_PIC);
		  assert(buf);
		  bmp.load(buf);
//		  bmp.set_clrkey(DEF_CLRKEY);
		  Window::set_r_border(&bmp);
		  
		  buf=g_fileloc.locate_file("win_top.bmp", TYPE_PIC);
		  assert(buf);
		  bmp.load(buf);
//		  bmp.set_clrkey(DEF_CLRKEY);
		  Window::set_top(&bmp);
		  
		  buf=g_fileloc.locate_file("win_bot.bmp", TYPE_PIC);
		  assert(buf);
		  bmp.load(buf);
//		  bmp.set_clrkey(DEF_CLRKEY);
		  Window::set_bot(&bmp);
		  
		  buf=g_fileloc.locate_file("win_bg.bmp", TYPE_PIC);
		  assert(buf);
		  bmp.load(buf);
//		  bmp.set_clrkey(DEF_CLRKEY);
		  Window::set_bg(&bmp);
}

/** void setup_searchpaths(void) 
 *
 * Sets the filelocators default searchpaths, this is OS specific
 *
 * Args: None
 * Returns: Nothing
 */
void setup_searchpaths(void) {
		  static File_Locator fileloc;
		 
		  char* buf;
		  char cmd[MAX_PATHL];
#ifdef WIN32
		  int i=0, n=0;
		  char drive[_MAX_DRIVE];
		  char dir[_MAX_DIR];
#endif

#ifndef	WIN32
		  buf=getenv("HOME");
		  strcpy(cmd, buf);
		  strcat(cmd, "/.widelands");
		  g_fileloc.add_searchdir(cmd, MAX_DIRS-2);
		  g_fileloc.set_def_writedir(MAX_DIRS-2);
		  
		  g_fileloc.add_searchdir(PKGDATADIR, MAX_DIRS-2);
#else

//#error	Code buggy!
		  buf=GetCommandLine();
		  while(buf[i]==' ' || buf[i]=='\"' ) i++;
		  while(buf[i]!='\"' && i<strlen(buf)) { cmd[n]=buf[i]; i++; n++; }
		  cmd[n]='\0';
		  _splitpath(cmd, drive, dir, NULL, NULL);
		  //a_MemFree(buf);

		  strcpy(cmd, drive);
		  strcat(cmd, dir);
//		  a_ResHandlerAddSearchDir(cmd);
		  g_fileloc.add_searchdir(cmd, MAX_DIRS-2);
#endif
}