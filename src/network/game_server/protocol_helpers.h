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

// Enable this to print a lot of information about what is written to
// ggzd game server module
//#define DEBGUG_WLGGZMETA_PROTO

#ifdef DEBGUG_WLGGZMETA_PROTO
#define DBG(x) x
#else
#define DBG(x)
#endif

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
		~WLGGZParameter()
		{
			if(pdata)
				delete static_cast<char *>(pdata);
		}
		WLGGZParameter():
			m_type(ggzdatatype_null), m_str(std::string("")), pdata(0) {}
		WLGGZParameter(int d):
			m_type(ggzdatatype_integer), m_i(d), m_str(std::string("")) {}
		WLGGZParameter(std::string d):
			m_type(ggzdatatype_string), m_str(d) {}
		WLGGZParameter(bool d):
			m_type(ggzdatatype_boolean), m_i(d), m_str(std::string("")) {}
		//WLGGZParameter(char d)
		
		WLGGZDataType get_type() { return m_type; }

		bool get_bool()          {return m_i; }
		int get_integer()        {return m_i; }
		//char get_char();
		std::string get_string() {return m_str; }
		int get_list_type()      { return m_list_type; }
		std::list<WLGGZParameter> get_list() { return m_list; }
		
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
		void set(int i, std::list<WLGGZParameter> list)
			{ m_list = list; m_list_type = i; m_type = ggzdatatype_list; }
		void set_raw(int size, void * p) { m_type=ggzdatatype_raw; pdata = p; }
		//void set(char d);

		bool is_string()  { return m_type==ggzdatatype_string; }
		bool is_integer() { return m_type==ggzdatatype_integer; }
		bool is_char()    { return m_type==ggzdatatype_char; }
		bool is_bool()    { return m_type==ggzdatatype_boolean; }
		bool is_list()    { return m_type==ggzdatatype_list; }
		
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
		int m_list_type;
		void * pdata;
};

typedef std::list<WLGGZParameter> WLGGZParameterList;

DBG(std::string level;)

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
		WLGGZ_writer(int fd, int list_type):
			m_sub(0),
			m_fd(fd),
			m_in_cmd(false)
		{
			type(list_type);
		}
		
		/*
		WLGGZ_writer(int fd, int list_type):
			m_sub(0),
			m_fd(fd),
			m_in_cmd(false)
		{}*/

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
			while(close_list());
			if (m_in_cmd) {

				DBG(level = level.substr(0, level.length() - 2);)
				DBG(
					std::cout <<
						"parameter_list_writer: " << level << "- flush write zero\n";)

				ggz_write_int(m_fd, 0);
			}
			m_in_cmd = false;
		}

		/**
		 * This sets the type of the parameter list to write. This must be called
		 * before writing parameters.
		 *
		 * @param t The type of the parameterlist. For example a 
		 *          @ref WLGGZNetworkOpcodes on top level. Other types
		 *          (@ref WLGGZGameInfo, @ref WLGGZPlayerType, @ref WLGGZGameType)
		 *          are only allowed as subtypes.
		 */
	private:
		void type(int t)
		{
			if (m_sub) {
				m_sub->type(t);
				return;
			}
			if (m_in_cmd)
				ggz_write_int(m_fd, 0);
			m_in_cmd = true;

			DBG(std::cout << "parameter_list_writer: " << level <<
				"- write parameterlist type code: " << t << std::endl;)
			DBG(level += "  ";)

			ggz_write_int(m_fd, t);
		}

	public:

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
				DBG(std::cout << "parameter_list_writer: " << level <<
					"- warning: not in cmd\n";)
				return *this;
			}
			if (m_sub) {
				*m_sub << d;
				return *this;
			}

			DBG(std::cout << "parameter_list_writer: " << level << "- write datatype integer\n";)
			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist value(int): " << d << std::endl;)

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
				DBG(std::cout << "parameter_list_writer: " << level << "- warning: not in cmd\n";)
				return *this;
			}
			if (m_sub) {
				*m_sub << d;
				return *this;
			}

			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist datatype string\n";)
			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist value(string): " << d << std::endl;;)

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
				DBG(std::cout << "parameter_list_writer: " << level << "- warning: not in cmd\n";)
				return *this;
			}
			if (m_sub) {
				*m_sub << d;
				return *this;
			}

			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist datatype char\n";)
			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist value(int)\n";)

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
				DBG(std::cout << "parameter_list_writer: " << level << "- warning: not in cmd\n";)
				return *this;
			}
			if (m_sub) {
				*m_sub << d;
				return *this;
			}

			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist datatype boolean\n";)
			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist value(int)\n";)

			ggz_write_int(m_fd, ggzdatatype_boolean);
			ggz_write_int(m_fd, d);
			return *this;
		}
		
		void write_raw_data(int size, void * data) {
			if (not m_in_cmd) {
				DBG(std::cout << "parameter_list_writer: " << level << "- warning: not in cmd\n";)
				return;
			}
			if (m_sub) {
				m_sub->write_raw_data(size, data);
				return;
			}

			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist datatype raw\n";)
			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist size\n";)
			DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist data\n";)

			ggz_write_int(m_fd, ggzdatatype_raw);
			ggz_write_int(m_fd, size);
			ggz_writen(m_fd, data, size);
		}

		void open_list(int list_type){
			if (not m_sub) {

				DBG(std::cout << "parameter_list_writer: " << level << 
					"- open a list - write datatype list\n";)
				DBG(level += "    ";)

				ggz_write_int(m_fd, ggzdatatype_list);
				m_sub = new WLGGZ_writer(m_fd, list_type);
			}
			else
				m_sub->open_list(list_type);
		}

		bool close_list() {
			if (m_sub) {
				if (not m_sub->close_list()) {
					// sublist closed nothing. So we close our list
					delete m_sub; // will flush on destroy
					m_sub = 0;
					DBG(level = level.substr(0, level.length() - 4);)
					DBG(std::cout << "parameter_list_writer: " << level << "- write parameterlist: close list\n";)
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
		bool m_in_cmd; ///< keep track if a parameter list is open to write to
};



#endif
