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

#ifndef __S__VARIABLE_MANAGER_H
#define __S__VARIABLE_MANAGER_H

#include <cassert>
#include <string>
#include <vector>

namespace Widelands {

/**
 * The Map Variable Manager makes sure that variables
 * in the map are watched accordingly.
 *
 * A Map variable is a user defined type, which can be changed with
 * triggers or events and displayed in strings. Also triggers can be
 * run when variables are over a certain value.
 *
 * Variables are mostly for mission objectives - simple stuff (like how many
 * buildings of a type should be build before triggering) doesn't use variables
 */

/**
 * First, the variables
 */
struct MapVariable {
      enum Type {
         MVT_INT,
         MVT_STRING,
		};

      MapVariable(bool t) {m_delete_protected = t;}
      virtual ~MapVariable() {}

	bool is_delete_protected() const {return m_delete_protected;}
	std::string const & get_name() const {return m_name;}
      void set_name(const char* name) {m_name = name;}

	virtual std::string get_string_representation() const = 0;
	virtual Type get_type() const = 0;

private:
      std::string   m_name;
      bool           m_delete_protected;
};

struct Int_MapVariable : public MapVariable {
      Int_MapVariable(bool t) : MapVariable(t) {m_value = 0;}

	Type get_type() const {return MVT_INT;}

	int32_t get_value() const {return m_value;}
      void set_value(int32_t t) {m_value = t;}
	std::string get_string_representation() const {
         char buffer[256];
         sprintf(buffer, "%i", m_value);
         return buffer;
		}

private:
      int32_t            m_value;
};

struct String_MapVariable : public MapVariable {
      String_MapVariable(bool t) : MapVariable(t) {m_value = "";}

	Type get_type() const {return MVT_STRING;}

	std::string const & get_value() const {return m_value;}
      void set_value(const char* t) {m_value = t;}
	std::string get_string_representation() const {return m_value;}

private:
      std::string            m_value;
};

/**
 * The manager himself.
 * This is mainly a wrapper, the function
 * could have also been implemented directly in the map.
 *
 * But it is better this way.
 */
struct MapVariableManager {
      MapVariableManager();
      ~MapVariableManager();

      /**
       * Register a new variable
       */
      bool register_new_variable(MapVariable*);

      /**
       * This prevents casting
       */
	Int_MapVariable    * get_int_variable   (const char * const name) const;
	String_MapVariable * get_string_variable(const char * const name) const;

      /**
       * Get a variable
       */
	MapVariable* get_variable(const char * const name) const;
	void delete_variable(MapVariable const &);

	typedef std::vector<MapVariable *> variable_vector;
	typedef variable_vector::size_type Index;
	Index get_nr_variables() const {return m_variables.size();}
	MapVariable & get_variable_by_nr(const Index i) const
	{assert(i < m_variables.size()); return *m_variables[i];}

private:
	variable_vector      m_variables;
};

};

#endif
