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
};

/**
 * This function reads a parameter list from a filedescriptor. This function
 * reads pairs of data type and data until the data type is zero. This reads
 * written with class WLGGZ_writer.
 *
 * @param fd The filedescriptor to read from
 * @return the list of the parameters
 */
std::list<WLGGZParameter> wlggz_read_parameter_list(int fd)
{
	std::list<WLGGZParameter> list;
	int datatype;
	ggz_read_int(fd, &datatype);
	//std::cout << "GGZ: read_int ("<< datatype <<") datatype\n";
	while(datatype)
	{	
		WLGGZParameter data;
		int d_i;
		char * d_str;
		char d_ch;
		
		switch(datatype)
		{
			case ggzdatatype_integer:
				ggz_read_int(fd, &d_i);
				//std::cout << "GGZ: read_parameter_list: read_integer ("<< d_i <<")\n";
				data.set(d_i);
				break;
			case ggzdatatype_char:
				ggz_read_char(fd, &d_ch);
				//std::cout << "GGZ: read_parameter_list: read_char ("<< d_ch <<")\n";
				data.set(d_ch);
			case ggzdatatype_string:
				ggz_read_string_alloc(fd, &d_str);
				//std::cout << "GGZ: read_parameter_string: read_string ("<< d_str <<")\n";
				data.set(std::string(strdup(d_str)));
				ggz_free(d_str);
				break;
			case ggzdatatype_boolean:
				ggz_read_int(fd, &d_i);
				//std::cout << "GGZ: read_parameter_list: read_boolean ("<< d_i <<")\n";
				data.set(static_cast<bool>(d_i));
				break;
			case ggzdatatype_raw:
				ggz_read_int(fd, &d_i);
				d_str = new char[d_i];
				ggz_readn(fd, d_str, d_i);
				std::cout << "GGZ: read_parameter_list: read_raw ("<< d_i <<" bytes)\n"<< 
					"WARNING: This is not implemented yet\n";
				delete[] d_str;
				break;
			default:
				std::cout << "GGZ: ERROR Received unknow data type" << std::endl;
		}
		list.push_back(data);
		ggz_read_int(fd, &datatype);
		//std::cout << "GGZ: read_int ("<< datatype <<") datatype\n";
	}
	//std::cout << "GGZ: read_parameter_list: leave\n";
	return list;
}

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
			if (m_in_cmd)
				ggz_write_int(m_fd, 0);
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
			if (not m_in_cmd)
				return *this;
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
			if (not m_in_cmd)
				return *this;
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
			if (not m_in_cmd)
				return *this;
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
			if (not m_in_cmd)
				return *this;
			ggz_write_int(m_fd, ggzdatatype_boolean);
			ggz_write_int(m_fd, d);
			return *this;
		}

	private:
		int m_fd;      ///< the filedescriptor to which all data is written
		bool m_in_cmd; ///< keep track if a parameter lsi is open to write to
};



#endif
