/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "options.h"
#include "parser.h"
#include "config.h"
#include "graphic.h"
#include "input.h"
#include "menuecommon.h"
#include "IntPlayer.h"


struct Options {
		  // Graphics
		  int xres;
		  int yres;
		  bool fullscreen;

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
static int parse_command_line(int, char**, Options*);
static int parse_conf_file(Options*);
static int write_conf_file(Options*);
static int consume_options(Options*);
static void show_usage(void);




/** static int write_conf_file(void)
 *
 * This overrites the old config file with a new one containing
 * the current options
 *
 * Args: o	Options to write
 * Returns: RET_OK or ERR_FAILED
 */
static int write_conf_file(Options* o)
{
	FileWrite fw;
	
	fw.Printf("# Widelands configfile, written by Version " VERSION "\n");
	fw.Printf("# Any comments to the game? Tell us on widelands-public@list.sourceforge.net\n");
	fw.Printf("\n");
	fw.Printf("# Graphics\n");
	// We really need to improve this anyway, so...
	// Apart from that, most of those options aren't used anyway
/*
		  f << "XRES" << "=" << o->xres << endl;
		  f << "YRES" << "=" << o->yres << endl;
		  f << "FULLSCREEN" << "=" << o->fullscreen << endl;
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
*/

	fw.Write(g_fs, "config");

	return RET_OK;
}


/** static int parse_conf_file(Options* o) 
 *
 * parses the config file using the parser class. Saving options in struct
 *
 * returns: RET_OK on success, ERR_FAILED otherwise
 */
static int parse_conf_file(Options* o)
{
	FileRead f;
	
	try {
		f.Open(g_fs, "config");
	} catch(...) {
		return RET_OK; // yes, this is normal on first startup
	}
	
	Parser p;

	p.register_int_opt("XRES", &o->xres);
	p.register_int_opt("YRES", &o->yres);
	p.register_bool_opt("FULLSCREEN", &o->fullscreen);

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
	int line = 1;
	while(f.ReadLine(buf, sizeof(buf))) {
		if(p.parse_line(buf)) {
			char output[1500];
		 
			sprintf(output, "Syntax error in config file (line %i): \"%s\"", line, buf);
			tell_user(output);
		}
		line++;
	}

	return RET_OK;
}

/** static int parse_command_line(uint argn, char** argc, Options* o)
 *
 * Parses the standart cmd line of the program
 *
 * Args: argn	number of args
 * 		argc	args (args[0] == name of exe)
 * 		o		Option struct to write stuff inside
 *
 * Returns: RET_OK on success
 */
static int parse_command_line(int argc, char** argv, Options* o)
{
	Parser p;
	char output[1024];
		  
	p.register_int_opt("--xres", &o->xres);
	p.register_int_opt("--yres", &o->yres);
	p.register_bool_opt("--fullscreen", &o->fullscreen);

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

	for(int i=1; i<argc; i++) {
		if(p.parse_line(argv[i])) {
			strcpy(output, "Parsing error on arg: ");
			strcat(output, argv[i]);
			tell_user(output);
			o->show_usage=true;
		}
	}

	return RET_OK;
}

/** void Handle_Options(int argn, char** argc)
 *
 * This function parses the config file and the cmdline,
 * checks the opts and send the different parts of the game
 * the options which are thought for them.
 *
 * Args: argn 	number of cmdline args
 * 		argc	cmdline args
 */
void handle_options(int argc, char** argv) {
		  Options o;

		  // Setting defaults
		  // Graphics
		  o.xres=640;
		  o.yres=480;
		  o.fullscreen=1;

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

			parse_conf_file(&o);

		  // Next, parse the comand line
		  // Errors don't matter. The user will be informed and a usage function will
		  // be called
		  parse_command_line(argc, argv, &o);

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

		  // Graphics
		  // set in game resolution (res for menues is fixed)
		  Interactive_Player::set_resolution(o->xres, o->yres);

		  if(o->fullscreen) {
					 g_gr.set_mode(0, 0, Graphic::MODE_FS);
		  } else {
					 g_gr.set_mode(0, 0, Graphic::MODE_WIN);
					 g_ip.grab_input(true);
		  }

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
		  err.open(o->stderr_file);

		  if(o->stdout_file[0]=='\0') {
					 strcpy(o->stdout_file, "stdout");
		  }
		  out.open(o->stdout_file);

		  return RET_OK;
}

/** static void show_usage(void)
 *
 * This functions finally prints the usage and ends the programm
 *
 * Args: None
 * Returns: Nothing
 */
static void show_usage(void)
{
	static const char help[] =
					 "Usage: widelands <option0>=<value0> ... <optionN>=<valueN>\n"
					 "Options:\n"
					 "Graphic:\n"
					 "\t--xres\t\t\tSet X resolution\n"
					 "\t--yres\t\t\tSet Y resolution\n"
					 "\t--fullscreen\t\tTurn fullscreen mode on/off\n\n"
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
					 "Bug reports? Suggestions? Check out the project website:\n"
					 "  http://www.sourceforge.net/f.net/projects/widelands\n"
					 "Hope you enjoy this game!\n"
					 "";

	tell_user(help);
}
