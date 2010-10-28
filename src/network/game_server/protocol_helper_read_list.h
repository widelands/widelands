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

#ifndef WLGGZEXCEPTION
#include "wlggz_exception.h"
#define WLGGZEXCEPTION parameterError()
#endif

/**
 * This function reads a parameter list from a filedescriptor. This function
 * reads pairs of data type and data until the data type is zero. This reads
 * written with class WLGGZ_writer.
 *
 * @param fd The filedescriptor to read from
 * @return the list of the parameters
 */
std::list<WLGGZParameter> wlggz_read_parameter_list(int fd, bool re = false)
{
	// prevent creating too deep lists. This is most probably a client error
	// or a denial of service attack.
	static int call_level;
	call_level++;
	if (not re)
		call_level = 0;
	if (call_level > 20) {
		call_level = 0;
		throw WLGGZEXCEPTION;
	}
	// prevent looping to often. The longest list currently are the statistic
	// vectors. 1000 samples should be enough for these.
	int length = 0;

	std::list<WLGGZParameter> list;
	int datatype;

	if( ggz_read_int(fd, &datatype) < 0 ){
		call_level--;
		throw WLGGZEXCEPTION;
	}

	while(datatype)
	{	
		WLGGZParameter data;
		int d_i;
		char * d_str;
		char d_ch;

		if (length++ > 1000) {
			call_level = 0;
			throw WLGGZEXCEPTION;
		}

		switch(datatype)
		{
			case ggzdatatype_integer:
				if (ggz_read_int(fd, &d_i) < 0) {
					call_level = 0;
					throw WLGGZEXCEPTION;
				}
				data.set(d_i);
				break;
			case ggzdatatype_char:
				if (ggz_read_char(fd, &d_ch) < 0) {
					call_level = 0;
					throw WLGGZEXCEPTION;
				}
				data.set(d_ch);
			case ggzdatatype_string:
				if (ggz_read_string_alloc(fd, &d_str) < 0) {
					call_level = 0;
					throw WLGGZEXCEPTION;
				}
				data.set(std::string(strdup(d_str)));
				ggz_free(d_str);
				break;
			case ggzdatatype_boolean:
				if (ggz_read_int(fd, &d_i) < 0) {
					call_level = 0;
					throw WLGGZEXCEPTION;
				}
				data.set(static_cast<bool>(d_i));
				break;
			case ggzdatatype_list:
				if (ggz_read_int(fd, &d_i) < 0) {
					call_level = 0;
					throw WLGGZEXCEPTION;
				}
				data.set(d_i, wlggz_read_parameter_list(fd, true));
				break;
			case ggzdatatype_raw:
				if (ggz_read_int(fd, &d_i) < 0) {
					call_level = 0;
					throw WLGGZEXCEPTION;
				}
				// do not accept raw data with more than 10KiB
				if (d_i > 10240) {
					call_level = 0;
					throw WLGGZEXCEPTION;
				}
				d_str = new char[d_i];
				if (ggz_readn(fd, d_str, d_i) < 0) {
					call_level = 0;
					throw WLGGZEXCEPTION;
				}
				// prevent getting lot of lists with raw data elements.
				length += 200;
				data.set_raw(d_i, d_str);
				break;
			default:
				std::cerr << "GGZ: ERROR Received unknow data type: " <<
				datatype << std::endl;
		}
		list.push_back(data);
		if (ggz_read_int(fd, &datatype) < 0) {
			call_level = 0;
			throw WLGGZEXCEPTION;
		}
	}
	call_level--;
	return list;
}