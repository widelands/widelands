/*
 * Copyright (C) 2002-2008 by the Widelands Development Team
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

#ifndef LANGUAGES_H
#define LANGUAGES_H

#include <string>
#include <cstring>

/*
 * This file simply contains the languages which are available as
 * translations in widelands.
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!!! NOTE This file must be saved as utf-8 encoded file !!!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

struct Languages {
	std::string name;
	std::string abbrev;
};


/*
 * Add your language below in alphabetical order (as far as possible). But keep
 * NONE as first entry. Also increase the NR_LANGUAGES variable by one
 */
#ifdef DEFINE_LANGUAGES  // defined in fullscreen_menu_options.cc
#define NR_LANGUAGES 16
static Languages available_languages[NR_LANGUAGES] = {
	{"Default system language", ""},
// EXTRACT BEGIN (leaves this line untouched)
	{"česky",      "cs_CZ"},
	{"Dansk",      "da_DK"},
	{"Deutsch",    "de_DE"},
	{"English",    "en_EN"},
	{"Español",    "es_ES"},
	{"Français",   "fr_FR"},
	{"Galego",     "gl_ES"},
	{"עברית",      "he_HE"},
	{"Magyar",     "hu_HU"},
	{"Nederlands", "nl_NL"},
	{"Polski",     "pl_PL"},
	{"Русский",    "ru_RU"},
	{"Suomi",      "fi_FI"},
	{"Slovensky",  "sk_SK"},
	{"Svenska",    "sv_SE"},
// EXTRACT END (leave this line untouched)
};
#endif

#endif
