/*
 * Copyright (C) 2010 The Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file network/game_server/protocol_helpers.h
 * @brief this header file contains some helper fuction for the widelands
 * metaserver protocol
 */

#ifndef WIDELANDS_PROTOCOL_HELPERS_H
#define WIDELANDS_PROTOCOL_HELPERS_H

#include "protocol.h"
#include <string>
#include <cstring>
#include <cstdarg>
#include <list>
#include <ggz.h>
#include <iostream>



class WLGGZParameter
{
	public:
		WLGGZParameter():
			m_type(ggzdatatype_null), m_str(std::string("")) {}
		WLGGZParameter(int d):
			m_type(ggzdatatype_integer), m_i(d), m_str(std::string("")) {}
		WLGGZParameter(std::string d):
			m_type(ggzdatatype_string), m_str(d) {}
		WLGGZParameter(bool d):
			m_type(ggzdatatype_boolean), m_i(d), m_str(std::string("")) {}
		//WLGGZParameter(char d)
		
		WLGGZDataType get_type() { return m_type; }

		bool get_bool() {return m_i; }
		int get_integer() {return m_i; }
		//char get_char();
		std::string get_string() {return m_str; }
		
		void set(bool d)
			{ m_type=ggzdatatype_boolean; m_i=d; }
		void set_bool(bool d)
			{ m_type=ggzdatatype_boolean; m_i=d; }
		void set(int d)
			{ m_type=ggzdatatype_integer; m_i=d; }
		void set_int(int d)
			{ m_type=ggzdatatype_integer; m_i=d; }
		void set(std::string d)
			{ m_type=ggzdatatype_string; m_str=d; }
		void set(std::list<WLGGZParameter> list)
			{ m_list = list; }
		//void set(char d);

		bool is_string() {return m_type==ggzdatatype_string; }
		bool is_integer() {return m_type==ggzdatatype_integer; }
		bool is_char() {return m_type==ggzdatatype_char; }
		bool is_bool() {return m_type==ggzdatatype_boolean; }
		
		bool is_valid() 
			{ return m_type == ggzdatatype_string or
				m_type == ggzdatatype_integer or
				m_type == ggzdatatype_char or
				m_type == ggzdatatype_boolean; }
		bool is_null() { return m_type==0; }
	private:
		WLGGZDataType m_type;
		int m_i;
		char m_ch;
		std::string m_str;
		std::list<WLGGZParameter> m_list;
};



/**
 * A class to write a list of parameters to a metaserver connection.
 * This writes the datatype before the data and automatically writes a zero
 * at the end of the list. Be sure to call flush or delete the object before
 * writing further data to the stream to be sure that the last zero is written.
 */
class WLGGZ_writer {
	public:
		/**
		 * The construtor takes the stream to write to as parameter
		 *
		 * @param fd The fd to which the data will be written
		 */
		WLGGZ_writer(int fd):
			m_sub(0),
			m_fd(fd),
			m_in_cmd(false)
		{}

		~WLGGZ_writer()
		{
			flush();
		}

		/**
		 * This function writes the zero after the last parameter of the list.
		 * Call this before writing data beside this class. This is called from
		 * the destructor.
		 */
		void flush()
		{
			// call close_list() until it returns false
			while(m_sub->close_list());
			if (m_in_cmd) {
				std::cout << "parameter_list_writer: flush write zero\n";
				ggz_write_int(m_fd, 0);
			}
			m_in_cmd = false;
		}

		/**
		 * This sets the type of the parameter list to write. This must be called
		 * before writing parameters.
		 *
		 * @param t The type of the parameterlist
		 */
		void type(int t)
		{
			if (m_in_cmd)
				ggz_write_int(m_fd, 0);
			m_in_cmd = true;
			ggz_write_int(m_fd, t);
		}

		/**
		 * Write a integer to the parameter list.
		 * type() must be calle before this.
		 *
		 * @param d The data to write to the parameter list
		 * @see type()
		 */
		WLGGZ_writer& operator<< (int d)
		{
			if (not m_in_cmd) {
				std::cout << "parameter_list_writer: warning: not in cmd\n";
				return *this;
			}
			if (m_sub) {
				*m_sub << d;
				return *this;
			}
			ggz_write_int(m_fd, ggzdatatype_integer);
			ggz_write_int(m_fd, d);
			return *this;
		}

		/**
		* Write a std::string to the parameter list.
		* type() must be calle before this.
		*
		* @param d The data to write to the parameter list
		* @see type()
		*/
		WLGGZ_writer& operator<< (const std::string &d)
		{
			if (not m_in_cmd) {
				std::cout << "parameter_list_writer: warning: not in cmd\n";
				return *this;
			}
			if (m_sub) {
				*m_sub << d;
				return *this;
			}
			ggz_write_int(m_fd, ggzdatatype_string);
			ggz_write_string(m_fd, d.c_str());
			return *this;
		}

		/**
		* Write a single character to the parameter list.
		* type() must be calle before this.
		*
		* @param d The data to write to the parameter list
		* @see type()
		*/
		WLGGZ_writer& operator<< (char d)
		{
			if (not m_in_cmd) {
				std::cout << "parameter_list_writer: warning: not in cmd\n";
				return *this;
			}
			if (m_sub) {
				*m_sub << d;
				return *this;
			}
			ggz_write_int(m_fd, ggzdatatype_char);
			ggz_write_char(m_fd, d);
			return *this;
		}

		/**
		* Write a bool to the parameter list.
		* type() must be calle before this.
		*
		* @param d The data to write to the parameter list
		* @see type()
		*/
		WLGGZ_writer& operator<< (bool d)
		{
			if (not m_in_cmd) {
				std::cout << "parameter_list_writer: warning: not in cmd\n";
				return *this;
			}
			if (m_sub) {
				*m_sub << d;
				return *this;
			}
			ggz_write_int(m_fd, ggzdatatype_boolean);
			ggz_write_int(m_fd, d);
			return *this;
		}

		void open_list(){
			if (not m_sub) {
				std::cout << "parameter_list_writer: open a list\n";
				ggz_write_int(m_fd, ggzdatatype_list);
				m_sub = new WLGGZ_writer(m_fd);
			}
			else
				m_sub->open_list();
		}

		bool close_list() {
			if (m_sub) {
				if (not m_sub->close_list()) {
					std::cout << "parameter_list_writer: close list\n";
					// sublist closed nothing. So we close our list
					delete m_sub; // will flush on destroy
					ggz_write_int(m_fd, 0); // termination of list
					return true;
				}
				// sublist closed something
				return true;
			} else  // nothing closed
				return false;
		}

	private:
		WLGGZ_writer * m_sub;
		int m_fd;      ///< the filedescriptor to which all data is written
		bool m_in_cmd; ///< keep track if a parameter lsi is open to write to
};



#endif
