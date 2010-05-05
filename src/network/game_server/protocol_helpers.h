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

#ifndef WIDELANDS_PROTOCOL_HELPERS_H
#define WIDELANDS_PROTOCOL_HELPERS_H

#include "protocol.h"
#include <string>
#include <cstring>
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
				delete d_str;
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

void wlggz_write_parameter_list(int fd, std::list<WLGGZParameter> l){
	std::list<WLGGZParameter>::iterator it = l.begin();
	while (it != l.end())
	{
		ggz_write_int(fd, it->get_type());
		//std::cout << "GGZ: write int (" << it->get_type() << ") parameter type" << std::endl;
		if(it->is_string())
		{
			ggz_write_string(fd, it->get_string().c_str());
			//std::cout << "GGZ: write string \"" << it->get_string().c_str() << "\" value" << std::endl;
		}
		else if (it->is_integer() or it->is_bool())
		{
			ggz_write_int(fd, it->get_integer());
			//std::cout << "GGZ: write int (" << it->get_integer() << ") value" << std::endl;
		}
		else 
			std::cout << "GGZ: ERROR Tried to write unhandled data type" << std::endl;
		it++;
	}
}

void wlggz_write(int fd, int type,std::list<WLGGZParameter> parlist)
{
	ggz_write_int(fd, type);
	//std::cout << "GGZ: write int (" << type << ") is a type" << std::endl;
	wlggz_write_parameter_list(fd, parlist);
	ggz_write_int(fd, 0);
	//std::cout << "GGZ: write int (0) end of type" << std::endl;
}

#endif
