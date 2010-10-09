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

#include "protocol_helpers.h"

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
	if( ggz_read_int(fd, &datatype) < 0 ){
		std::cout << "wlggz_read_parameter_list: ERROR on read\n";
		return list;
	}
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
			case ggzdatatype_list:
				data.set(wlggz_read_parameter_list(fd));
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
				std::cout << "GGZ: ERROR Received unknow data type: " <<
				datatype << std::endl;
		}
		list.push_back(data);
		ggz_read_int(fd, &datatype);
		//std::cout << "GGZ: read_int ("<< datatype <<") datatype\n";
	}
	//std::cout << "GGZ: read_parameter_list: leave\n";
	return list;
}