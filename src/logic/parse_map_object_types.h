/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#define PARSE_MAP_OBJECT_TYPES_BEGIN(kind)                                    \
   if (Section * const section = root_conf.get_section(kind " types")) {      \
      while (Section::Value const * const v = section->get_next_val()) {      \
         char const * const     _name = v->get_name  ();                      \
         char const * const _descname = v->get_string();                      \
         if (names.count(_name))                                              \
            throw game_data_error                                             \
               ("object name \"%s\" is already used", _name);                 \
         names.insert(_name);                                                 \
         path += _name;                                                       \
         path += "/conf";                                                     \
         try {                                                                \
            Profile prof(path.c_str(), "global");                             \
            path.resize(path.size() - strlen("conf"));                        \
            Section & global_s = prof.get_safe_section("global");             \

#define PARSE_MAP_OBJECT_TYPES_END                                            \
            prof.check_used();                                                \
         } catch (const std::exception & e) {                                 \
            throw game_data_error                                             \
               ("%s=\"%s\": %s", _name, _descname, e.what());                 \
         }                                                                    \
         path.resize(base_path_size);                                         \
      }                                                                       \
   }
