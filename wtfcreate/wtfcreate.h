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

#ifndef __S__WTFCREATE_H
#define __S__WTFCREATE_H

#define CONF_NAME		"conf"
#define WTF_EXT			".wtf"

#ifdef WIN32
 #define SEPERATOR		'\\'
 #define SSEPERATOR	"\\"
#else
 #define SEPERATOR		'/'
 #define SSEPERATOR	"/"
#endif

#define PICS_DIR     "pics"  SSEPERATOR
#define BOBS_DIR     "bobs"  SSEPERATOR
#define WARES_DIR    "wares" SSEPERATOR
#define SOLDIERS_DIR "soldiers" SSEPERATOR
#define WORKERS_DIR  "workers" SSEPERATOR
#define BUILDINGS_DIR "buildings" SSEPERATOR

#define MENU_PIC_BL    24
#define REGENT_PB_W    70
#define REGENT_PB_H    100
#define REGENT_PS_W    45
#define REGENT_PS_H    70

extern char g_dirname[];


#endif /* __S__WTFCREATE_H */
