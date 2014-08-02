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

#include "base/time_string.h"

#include <algorithm>
#include <cassert>
#include <ctime>
#include <string>

#include <boost/format.hpp>
#include <stdint.h>

#include "base/i18n.h"

namespace  {
char timestring_buffer[] = "YYYY-MM-DDThh.mm.ss"; //  ':' is not a valid file name character for FAT FS
char gamestringbuffer[] = "000:00:00";
}  // namespace

char * timestring() {
	time_t t;
	time(&t);
	struct tm const * const t_components = localtime(&t);
	{
		uint16_t year = 1900 + t_components->tm_year; //  years start at 1900
		assert(year < 9999);
		timestring_buffer [0] = '0' + year / 1000;
		year %= 1000;
		timestring_buffer [1] = '0' + year /  100;
		year %=  100;
		timestring_buffer [2] = '0' + year /   10;
		year %=   10;
		timestring_buffer [3] = '0' + year;
	}
	{
		uint16_t mon = 1 + t_components->tm_mon; //  months start at 0
		assert(mon  <=   99);
		timestring_buffer [5] = '0' + mon  /   10;
		mon  %=   10;
		timestring_buffer [6] = '0' + mon;
	}
	{
		uint16_t mday = t_components->tm_mday;
		assert(mday <=   99);
		timestring_buffer [8] = '0' + mday /   10;
		mday %=   10;
		timestring_buffer [9] = '0' + mday;
	}
	{
		uint16_t hour = t_components->tm_hour;
		assert(hour <=   99);
		timestring_buffer[11] = '0' + hour /   10;
		hour %=   10;
		timestring_buffer[12] = '0' + hour;
	}
	{
		uint16_t min = t_components->tm_min;
		assert(min  <=   99);
		timestring_buffer[14] = '0' + min  /   10;
		min  %=   10;
		timestring_buffer[15] = '0' + min;
	}
	{
		uint16_t sec = t_components->tm_sec;
		assert(sec  <=   99);
		timestring_buffer[17] = '0' + sec  /   10;
		sec  %=   10;
		timestring_buffer[18] = '0' + sec;
	}
	return timestring_buffer;
}

namespace  {
std::string localize_month(int8_t month) {
	std::string fallback = std::to_string(month);
	switch (month) {
		case 1:
			/** TRANSLATORS: January */
			return _("Jan");
		case 2:
			/** TRANSLATORS: February */
			return _("Feb");
		case 3:
			/** TRANSLATORS: March */
			return _("Mar");
		case 4:
			/** TRANSLATORS: April */
			return _("Apr");
		case 5:
			/** TRANSLATORS: May */
			return _("May");
		case 6:
			/** TRANSLATORS: June */
			return _("Jun");
		case 7:
			/** TRANSLATORS: July */
			return _("Jul");
		case 8:
			/** TRANSLATORS: August */
			return _("Aug");
		case 9:
			/** TRANSLATORS: September */
			return _("Sep");
		case 10:
			/** TRANSLATORS: October */
			return _("Oct");
		case 11:
			/** TRANSLATORS: November */
			return _("Nov");
		case 12:
			/** TRANSLATORS: December */
			return _("Dec");
		default:
			return fallback;
	}
}
}


// Locale-dependent formatting for datetime-based filenames.
std::string localize_timestring(std::string timestring) {

	std::string result = "";

	// Do some formatting if this is a string of the type "YYYY-MM-DDThh.mm.ss"
	// check separators
	if (timestring.length() >= sizeof(timestring_buffer) - 1 &&
		 timestring.compare(4, 1, "-") == 0 &&
		 timestring.compare(7, 1, "-") == 0 &&
		 timestring.compare(10, 1, "T") == 0 &&
		 timestring.compare(13, 1, ".") == 0 &&
		 timestring.compare(16, 1, ".") == 0) {

		std::string year = timestring.substr(0, 4);
		std::string month = timestring.substr(5, 2);
		std::string day = timestring.substr(8, 2);
		std::string hour = timestring.substr(11, 2);
		std::string minute = timestring.substr(14, 2);
		std::string second = timestring.substr(17, 2);

		// check digits
		if (std::all_of(year.begin(), year.end(), ::isdigit) &&
			 std::all_of(month.begin(), month.end(), ::isdigit) &&
			 std::all_of(day.begin(), day.end(), ::isdigit) &&
			 std::all_of(hour.begin(), hour.end(), ::isdigit) &&
			 std::all_of(minute.begin(), minute.end(), ::isdigit) &&
			 std::all_of(second.begin(), second.end(), ::isdigit)) {

			month = localize_month(stoi(month));

			/** TRANSLATORS: Date format for filenames on load game screens. YYYY Mon DD hh:mm:ss */
			result = (boost::format(_("%1% %2% %3% %4%:%5%:%6%"))
						 % day % month % year
						 % hour % minute % second).str();

			if (timestring.length() > sizeof(timestring_buffer) - 1) {
				result.append(timestring.substr(19));
			}
		} else {
			result = timestring;
		}
	} else {
		result = timestring;
	}
	return result;
}


char * gametimestring_leading_zeros(uint32_t gametime)
{
	uint32_t time = gametime / 1000;
	gamestringbuffer[8] = '0' +  time        % 10;
	gamestringbuffer[7] = '0' + (time /= 10) %  6;
	gamestringbuffer[5] = '0' + (time /=  6) % 10;
	gamestringbuffer[4] = '0' + (time /= 10) %  6;
	gamestringbuffer[2] = '0' + (time /=  6) % 10;
	gamestringbuffer[1] = '0' + (time /= 10) % 10;
	gamestringbuffer[0] = '0' + (time /= 10);
	return gamestringbuffer;
}

char * gametimestring(uint32_t gametime)
{
	// update buffer
	gametimestring_leading_zeros(gametime);

	// remove leading 0s
	int8_t returnindex = 0;
	if (gamestringbuffer[0] == '0')
	{
		returnindex++;
		if (gamestringbuffer[1] == '0')
		{
			returnindex++;
			if (gamestringbuffer[2] == '0')
			{
				returnindex = returnindex + 2;
			}
		}
	}
	return &gamestringbuffer[returnindex];
}
