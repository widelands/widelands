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

// 2002-02-10	sft+	added config-existing check in handle_options

#include "options.h"
#include "parser.h"
#include "myfile.h"
#include "errors.h"
#include "fileloc.h"
#include "config.h"
#include "output.h"
#include "os.h"
#include "graphic.h"
#include "input.h"
#include <iostream.h>
#include <string.h>
#include <stdlib.h>

struct Options {
		  // Graphics
		  int xres;
		  int yres;
		  bool fullscreen;

		  // Dirs
		  char searchdir0[MAX_OPT_LENGTH];
		  char searchdir1[MAX_OPT_LENGTH];
		  char searchdir2[MAX_OPT_LENGTH];
		  char txtsdir[MAX_OPT_LENGTH];
		  char picsdir[MAX_OPT_LENGTH];
		  char bobsdir[MAX_OPT_LENGTH];
		  char tribesdir[MAX_OPT_LENGTH];
		  char worldsdir[MAX_OPT_LENGTH];
		  char campaignsdir[MAX_OPT_LENGTH];
		  char mapsdir[MAX_OPT_LENGTH];
		  char musicsdir[MAX_OPT_LENGTH];
		  char effectsdir[MAX_OPT_LENGTH];
		  
		  // Gameplay
		  bool swapmouse;
		  int  mousespeed;
		  char playercolor[MAX_OPT_LENGTH];

		  /* Keyboard still to define */

		  // Network
		  char player[MAX_OPT_LENGTH];
		  char lastserver[MAX_OPT_LENGTH];

		  // Sound
		  bool usemusic;
		  bool useeffects;
		  int  music_vol;
		  int  effects_vol;
		  char snddev[MAX_OPT_LENGTH];

		  // Cmdline stuff
		  bool show_version;
		  bool show_usage;
		  
		  // Debug
		  char stderr_file[MAX_OPT_LENGTH];
		  char stdout_file[MAX_OPT_LENGTH];

};

// vars
ofstream out, err;

// Declaratiosn
static int parse_command_line(unsigned int, char**, Options*);
static int parse_conf_file(Ascii_file&, Options*);
static int write_conf_file(Options*);
static void tell_user(const char*);
static int consume_options(Options*);
static void show_usage(void);

/** static void tell_user(const char* str) 
 *
 * This functions makes sure that the user gets informed of what's going on 
 * It's used since this information should reach the user directly 
 * and not been redirected to stderr. 
 *
 * Args: str	String to give to user
 * Returns: Nothing
 */
static void tell_user(const char* str) {
#ifdef WINDOWS
#error 	tell_user function not defined for windows yet!!
#else
		  cout << str << endl;
#endif
		  
}


/** static int write_conf_file(void)
 *
 * This overrites the old config file with a new one containing
 * the current options
 *
 * Args: o	Options to write
 * Returns: RET_OK or ERR_FAILED
 */
static int write_conf_file(Options* o) {
		  const char* buf;

		  buf=g_fileloc.get_new_filename("config");

		  ofstream f(buf);
		  
		  f << "# Widelands configfile, written by Version " << VERSION << endl;
		  f << "# Any comments to the game? mailto:SirVer@gmx.de" << endl;
		  f << endl;
		  f << "# Graphics" << endl;
		  f << "XRES" << "=" << o->xres << endl;
		  f << "YRES" << "=" << o->yres << endl;
		  f << "FULLSCREEN" << "=" << o->fullscreen << endl;
		  f << "" << endl;
		  f << "# Directorys" << endl;
		  f << "SEARCHDIR0" << "=\"" << o->searchdir0 << "\"" << endl;
		  f << "SEARCHDIR1" << "=\"" << o->searchdir1 << "\"" << endl;
		  f << "SEARCHDIR2" << "=\"" << o->searchdir2 << "\"" << endl;
		  f << "TXTSDIR" << "=\"" << o->txtsdir << "\"" << endl;
		  f << "PICSDIR" << "=\"" << o->picsdir << "\"" << endl;
		  f << "BOBSDIR" << "=\"" << o->bobsdir << "\"" << endl;
		  f << "TRIBESDIR" << "=\"" << o->tribesdir << "\"" << endl;
		  f << "WORLDSDIR" << "=\"" << o->worldsdir << "\"" << endl;
		  f << "CAMPAIGNSDIR" << "=\"" << o->campaignsdir << "\"" << endl;
		  f << "MAPSDIR" << "=\"" << o->mapsdir << "\"" << endl;
		  f << "MUSICSDIR" << "=\"" << o->musicsdir << "\"" << endl;
		  f << "EFFECTSDIR" << "=\"" << o->effectsdir << "\"" << endl;
		  f << "" << endl;
		  f << "# Gameplay" << endl;
		  f << "SWAPMOUSE" << "=" << o->swapmouse << endl;
		  f << "MOUSESPEED" << "=" << o->mousespeed << endl;
		  f << "PLAYERCOLOR" << "=\"" << o->playercolor << "\"" << endl;
		  f << "" << endl;

		  // Keyboard still missing" << "="
		  f << "# Network" << endl;
		  f << "PLAYER" << "=\"" << o->player << "\"" << endl;
		  f << "LASTSERVER" << "=\"" << o->lastserver << "\"" << endl;
		  f << "" << endl;
		  f << "# Sound" << endl;
		  f << "USEMUSIC" << "=" << o->usemusic << endl;
		  f << "USEEFFECTS" << "=" << o->useeffects << endl;
		  f << "VOLUME_MUSIC" << "=" << o->music_vol << endl;
		  f << "VOLUME_EFFECTS" << "=" << o->effects_vol << endl;
		  f << "SNDDEV" << "=\"" << o->snddev << "\"" << endl;
		  f << "" << endl;
		  f << "# Debug" << endl;
		  f << "STDERR" << "=\"" << o->stderr_file << "\"" << endl;
		  f << "STDOUT" << "=\"" << o->stdout_file << "\"" << endl;
		  f.close();
		  
		  return RET_OK;
}


/** static int parse_conf_file(Ascii_file &f, Options* o) 
 *
 * parses the config file using the parser class. Saving options in struct
 *
 * Args: f	open file to parse
 * 		o  Optionsstruct to write options inside
 * returns: RET_OK on success, ERR_FAILED otherwise
 */
static int parse_conf_file(Ascii_file &f, Options* o) {
		  if(f.get_state() != File::OPEN) return ERR_FAILED; 
		  
		  Parser p;

		  p.register_int_opt("XRES", &o->xres);
		  p.register_int_opt("YRES", &o->yres);
		  p.register_bool_opt("FULLSCREEN", &o->fullscreen);

		  p.register_string_opt("SEARCHDIR0", o->searchdir0);
		  p.register_string_opt("SEARCHDIR1", o->searchdir1);
		  p.register_string_opt("SEARCHDIR2", o->searchdir2);
		 
		  p.register_string_opt("TXTSDIR", o->txtsdir);
		  p.register_string_opt("PICSDIR", o->picsdir);
		  p.register_string_opt("BOBSDIR", o->bobsdir);
		  p.register_string_opt("TRIBESDIR", o->tribesdir);
		  p.register_string_opt("WORLDSDIR", o->worldsdir);
		  p.register_string_opt("CAMPAIGNSDIR", o->campaignsdir);
		  p.register_string_opt("MAPSDIR", o->mapsdir);
		  p.register_string_opt("MUSICSDIR", o->musicsdir);
		  p.register_string_opt("EFFECTSDIR", o->effectsdir);

		  p.register_bool_opt("SWAPMOUSE", &o->swapmouse);
		  p.register_int_opt("MOUSESPEED", &o->mousespeed);
		  p.register_string_opt("PLAYERCOLOR", o->playercolor);
		  
		  p.register_string_opt("PLAYER", o->player);
		  p.register_string_opt("LASTSERVER", o->lastserver);

		  p.register_bool_opt("USEMUSIC", &o->usemusic);
		  p.register_bool_opt("USEEFFECTS", &o->useeffects);
		 
		  p.register_int_opt("VOLUME_MUSIC", &o->music_vol);
		  p.register_int_opt("VOLUME_EFFECTS", &o->effects_vol);
		  p.register_string_opt("SNDDEV", o->snddev);

		  p.register_string_opt("STDERR", o->stderr_file);
		  p.register_string_opt("STDOUT", o->stdout_file);
					 
		  char buf[1024];
		  buf[0]='\0';
		  int line=1;
		  do {
					 f.read_line(buf, 1024);
					 if(f.get_state() != File::END_OF_FILE) {
								if(p.parse_line(buf)) {
										  char output[1500];
		 
										  sprintf(output, "Syntax error in config file (line %i): \"%s\"", line, buf);
										  tell_user(output);
								}
								line++;
					 }
		  }  while(f.get_state() != File::END_OF_FILE ); 

		  return RET_OK;
}

/** static int parse_command_line(unsigned int argn, char** argc, Options* o) 
 *
 * Parses the standart cmd line of the program
 *
 * Args: argn	number of args
 * 		argc	args (args[0] == name of exe)
 * 		o		Option struct to write stuff inside
 * 
 * Returns: RET_OK on success
 */
static int parse_command_line(unsigned int argn, char** argc, Options* o) {
		  Parser p;
		  char output[1024];
		  
		  p.register_int_opt("--xres", &o->xres);
		  p.register_int_opt("--yres", &o->yres);
		  p.register_bool_opt("--fullscreen", &o->fullscreen);

		  p.register_string_opt("--searchdir0", o->searchdir0);
		  p.register_string_opt("--searchdir1", o->searchdir1);
		  p.register_string_opt("--searchdir2", o->searchdir2);
		 
		  p.register_string_opt("--txtsdir", o->txtsdir);
		  p.register_string_opt("--picsdir", o->picsdir);
		  p.register_string_opt("--bobsdir", o->bobsdir);
		  p.register_string_opt("--tribesdir", o->tribesdir);
		  p.register_string_opt("--worldsdir", o->worldsdir);
		  p.register_string_opt("--campaignsdir", o->campaignsdir);
		  p.register_string_opt("--mapsdir", o->mapsdir);
		  p.register_string_opt("--musicsdir", o->musicsdir);
		  p.register_string_opt("--effectsdir", o->effectsdir);

		  p.register_bool_opt("--swapmouse", &o->swapmouse);
		  p.register_int_opt("--mousespeed", &o->mousespeed);
		  p.register_string_opt("--playercolor", o->playercolor);
		  
		  p.register_string_opt("--player", o->player);
		  p.register_string_opt("--lastserver", o->lastserver);

		  p.register_bool_opt("--usemusic", &o->usemusic);
		  p.register_bool_opt("--useeffects", &o->useeffects);
		 
		  p.register_int_opt("--volume_music", &o->music_vol);
		  p.register_int_opt("--volume_effects", &o->effects_vol);
		  p.register_string_opt("--snddev", o->snddev);

		  p.register_string_opt("--stderr", o->stderr_file);
		  p.register_string_opt("--stdout", o->stdout_file);

		  p.register_bool_opt("--help", &o->show_usage);
		  p.register_bool_opt("--version", &o->show_version);
		  
		  for(unsigned int i=1; i<argn; i++) {
					 if(p.parse_line(argc[i])) {
								strcpy(output, "Parsing error on arg: ");
								strcat(output, argc[i]);
								tell_user(output);
								o->show_usage=true;
					 }
		  }

		  
		  return RET_OK;
}

/** void Handle_Options(unsigend int argn, char** argc)
 *
 * This function parses the config file and the cmdline,
 * checks the opts and send the different parts of the game
 * the options which are thought for them.
 *
 * Args: argn 	number of cmdline args
 * 		argc	cmdline args
 */
void handle_options(unsigned int argn, char** argc) {
		  Ascii_file f;
		  Options o;

		  // Setting defaults
		  // Graphics
		  o.xres=640;
		  o.yres=480;
		  o.fullscreen=1;

		  // Dirs
		  strcpy(o.searchdir0, "");
		  strcpy(o.searchdir1, "");
		  strcpy(o.searchdir2, "");
		  strcpy(o.txtsdir   , "txts");
		  strcpy(o.picsdir   , "pics");
		  strcpy(o.bobsdir   , "bobs");
		  strcpy(o.tribesdir , "tribes");
		  strcpy(o.worldsdir , "worlds");
		  strcpy(o.campaignsdir , "campaigns");
		  strcpy(o.mapsdir 	 , "maps");
		  strcpy(o.musicsdir , "musics");
		  strcpy(o.effectsdir, "effects");
		  
		  // Gameplay
		  o.swapmouse=0;
		  o.mousespeed=100;
		  strcpy(o.playercolor, "Blue");

		  // Network
		  strcpy(o.player, "");
		  strcpy(o.lastserver, "");

		  // Sound
		  o.usemusic=0;
		  o.useeffects=0;
		  o.music_vol=100;
		  o.effects_vol=100;
		  strcpy(o.snddev, "");

		  // Cmdline stuff
		  o.show_version=0;
		  o.show_usage=0;
		  
		  // Debug
		  strcpy(o.stderr_file, "stderr");
		  strcpy(o.stdout_file, "stdout");

		  const char* buf;
		  buf=g_fileloc.locate_file("config");
		  if (buf)
		  	  f.open(buf, File::READ);

		  if(f.get_state() != File::OPEN) {
					 // We couldn't open the config file, so we write a default one
					 write_conf_file(&o);
		  } else {
					 // Else, parse the file
					 // Errors here don't matter, the user will be informed
					 // and on exit a new conf file will be written
					 parse_conf_file(f, &o);
		  }
		  
		  // Next, parse the comand line
		  // Errors don't matter. The user will be informed and a usage function will
		  // be called
		  parse_command_line(argn, argc, &o);

		  // Options struct is now set up! use them now
		  consume_options(&o);
}

/** static int consume_options(Options* o) 
 *
 * This functions finally send the options to the parts of the game they belong to
 *
 * Args: o 	options to use
 * Returns: RET_OK
 */
static int consume_options(Options* o) {
		  static Graphic g; // global graphic class. here defined for whole game
		  static Input myip; // global input class. here defined for whole game

		  // Graphics, TODO
		  if(o->fullscreen) {
					 g_gr.set_mode(o->xres, o->yres, Graphic::MODE_FS);
		  } else {
					 g_gr.set_mode(o->xres, o->yres, Graphic::MODE_WIN);
					 g_ip.grab_input(true);
		  }
		  g_ip.set_max_cords(o->xres, o->yres);

		  // Dirs
		  g_fileloc.add_searchdir(o->searchdir0, 0);
		  g_fileloc.add_searchdir(o->searchdir1, 1);
		  g_fileloc.add_searchdir(o->searchdir2, 2);
		  g_fileloc.register_subdir(TYPE_TEXT, o->txtsdir); 
		  g_fileloc.register_subdir(TYPE_PIC, o->picsdir);
		  g_fileloc.register_subdir(TYPE_BOB, o->bobsdir);
		  g_fileloc.register_subdir(TYPE_TRIBE, o->tribesdir); 
		  g_fileloc.register_subdir(TYPE_WORLD, o->worldsdir);
		  g_fileloc.register_subdir(TYPE_CAMPAIGN, o->campaignsdir);
		  g_fileloc.register_subdir(TYPE_MAP, o->mapsdir);
		  g_fileloc.register_subdir(TYPE_MUSIC, o->musicsdir);
		  g_fileloc.register_subdir(TYPE_EFFECT, o->effectsdir);

		  // Gameplay 
		  g_ip.swap_buttons(o->swapmouse);
		  g_ip.set_mouse_speed(o->mousespeed);
		 
		  // Player color TODO


		  // Network TODO

		  // Sound 
		  if(o->usemusic || o->useeffects) {
					 tell_user("Sound is not implemented in this release. Please turn this options off!");
					 o->usemusic=o->useeffects=0;
		  }

		  // Cmdline stuff
		  if(o->show_version) {
					 tell_user(VERSION);
					 exit(0);
		  }

		  if(o->show_usage) {
					 show_usage();
					 exit(0);
		  }
		  
		  // Debug
		  if(o->stderr_file[0]=='\0') {
					 strcpy(o->stderr_file, "stderr");
		  }
		  const char* buf=g_fileloc.get_new_filename(o->stderr_file);
		  err.open(buf);
		  
		  if(o->stdout_file[0]=='\0') {
					 strcpy(o->stdout_file, "stdout");
		  }
		  buf=g_fileloc.get_new_filename(o->stdout_file);
		  out.open(buf);
		  
		  return RET_OK;
}

/** static void show_usage(void) 
 *
 * This functions finally prints the usage and ends the programm
 *
 * Args: None
 * Returns: Nothing
 */
static void show_usage(void) {
		  char help[] = 
					 "Usage: widelands <option0>=<value0> ... <optionN>=<valueN>\n"
					 "Options:\n"
					 "Graphic:\n"
					 "\t--xres\t\t\tSet X resolution\n"
					 "\t--yres\t\t\tSet Y resolution\n"
					 "\t--fullscreen\t\tTurn fullscreen mode on/off\n\n"
					 "\n"
					 "Directorys:\n"
					 "\t--searchdir0\t\tFirst additional searchdir\n"
					 "\t--searchdir1\t\tSecond additional searchdir\n"
					 "\t--searchdir2\t\tThird additional searchdir\n"
					 "\t--txtsdir\t\tSubdir to look for txt resources in\n"
					 "\t--picsdir\t\tSubdir to look for picture resources in\n"
					 "\t--bobsdir\t\tSubdir to look for bobs resources in\n"
					 "\t--tribesdir\t\tSubdir to look for tribes resources in\n"
					 "\t--worldsdir\t\tSubdir to look for worlds resources in\n"
					 "\t--campaignsdir\t\tSubdir to look for campaign resources in\n"
					 "\t--mapsdir\t\tSubdir to look for maps resources in\n"
					 "\t--musicsdir\t\tSubdir to look for music resources in\n"
					 "\t--effectsdir\t\tSubdir to look for sound-effect resources in\n"
					 "\n"
					 "Gameplay:\n"
					 "\t--swapmouse\t\tTurns mouse swapping on/off\n"
					 "\t--mousespeed\t\tSets mouse movement speed in percent\n"
					 "\t--playercolor\t\tSets the player color\n"
					 "\n"
					 "Network:\n"
					 "\t--player\t\tPlayer name to use\n"
					 "\t--lastserver\t\tServer to connect to\n"
					 "\n"
					 "Sound:\n"
					 "\t--usemusic\t\tTurns music on/off\n"
					 "\t--useeffects\t\tTurn effects on/off\n"
					 "\t--volume_music\t\tSets music volume in percent\n"
					 "\t--volume_effects\tSets effect volume in percent\n"
					 "\t--snddev\t\tSound device to use for output\n"
					 "\n"
					 "Debug:\n"
					 "\t--stderr\t\tFile to redirect stderr to\n"
					 "\t--stdout\t\tFile to redirect stdout to\n"
					 "\n"
					 "\t--help\t\t\tShow this help\n"
					 "\t--version\t\tShow version\n"
					 "\n"
					 "Bug reports? Suggestions? mailto:SirVer@gmx.de\n"
					 "Hope you enjoy this game!\n"
					 "";
		  
		  tell_user(help);
}
