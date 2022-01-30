/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "base/string.h"

#include <memory>

std::string to_lower(const std::string& str) {
	std::string s;
	for (const char* c = str.c_str(); *c; ++c) {
		s.push_back(tolower(*c));
	}
	return s;
}

bool iequals(const std::string& str1, const std::string& str2) {
	const char* a = str1.c_str();
	const char* b = str2.c_str();
	for (; *a && *b; ++a, ++b) {
		if (tolower(*a) != tolower(*b)) {
			return false;
		}
	}
	return (*a == '\0' && *b == '\0');
}

bool starts_with(const std::string& str, const std::string& test, const bool case_sensitive) {
	const char* a = str.c_str();
	const char* b = test.c_str();
	for (; *a && *b; ++a, ++b) {
		if (case_sensitive ? (*a != *b) : (tolower(*a) != tolower(*b))) {
			return false;
		}
	}
	return *b == '\0';
}

bool ends_with(const std::string& str, const std::string& test, const bool case_sensitive) {
	const char* a = str.c_str();
	size_t len_a = 0;
	// Go to the end of the `str` string
	for (; *a; ++a, ++len_a) {
	}

	// Go to the end of the `test` string
	const char* b = test.c_str();
	size_t len_b = 0;
	for (; *b; ++b) {
		++len_b;
		if (len_b > len_a) {
			return false;
		}
	}

	// Step back, one char at a time
	for (; len_b; --len_b) {
		--b;
		--a;
		if (case_sensitive ? (*a != *b) : (tolower(*a) != tolower(*b))) {
			return false;
		}
	}

	return true;
}

void split(std::vector<std::string>& result, const std::string& str, const std::set<char>& set) {
	result.clear();
	std::string cur;

	for (const char* c = str.c_str(); *c; ++c) {
		if (set.count(*c)) {
			result.push_back(cur);
			cur.clear();
		} else {
			cur.push_back(*c);
		}
	}

	result.push_back(cur);
}

void trim(std::string& str, const bool remove_leading, const bool remove_trailing) {
	if (remove_leading) {
		const size_t pos = str.find_first_not_of(' ');
		if (pos == std::string::npos) {
			str.clear();
			return;
		}
		str.erase(0, pos);
	}

	if (remove_trailing) {
		size_t len = str.size();
		while (len > 0 && str[len - 1] == ' ') {
			str.pop_back();
			--len;
		}
	}
}

bool replace_first_or_last(std::string& str,
                           const std::string& f,
                           const std::string& r,
                           bool first) {
	const size_t pos = (first ? str.find(f) : str.rfind(f));
	if (pos == std::string::npos) {
		return false;
	}

	str.replace(pos, f.size(), r);
	return true;
}

void replace_all(std::string& str, const std::string& f, const std::string& r) {
	const size_t sf = f.size();
	const size_t sr = r.size();
	for (size_t pos = 0;;) {
		pos = str.find(f, pos);
		if (pos == std::string::npos) {
			return;
		}
		str.replace(pos, sf, r);
		pos += sr;
	}
}
