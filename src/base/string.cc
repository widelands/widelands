/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#include "base/string.h"

std::vector<std::string> split(const std::string& str, const std::set<char>& set) {
	std::vector<std::string> result;
	std::string cur;

	for (const char* c = str.c_str(); *c; ++c) {
		if (set.contains(*c)) {
			result.push_back(cur);
			cur.clear();
		} else {
			cur.push_back(*c);
		}
	}

	result.push_back(cur);
	return result;
}

std::string trim(std::string str, const bool remove_leading, const bool remove_trailing) {
	if (remove_leading) {
		const size_t pos = str.find_first_not_of(" ");
		if (pos == std::string::npos) {
			return std::string();
		}
		str = str.substr(pos);
	}

	if (remove_trailing) {
		size_t len = str.size();
		while (len > 0 && str[len - 1] == ' ') {
			str.pop_back();
			--len;
		}
	}

	return str;
}

std::string join(const std::vector<std::string>& words, const std::string& separator) {
	if (words.empty()) {
		return std::string();
	}

	std::string str = words[0];
	const size_t n = words.size();

	for (size_t i = 1; i < n; ++i) {
		str += separator;
		str += words[i];
	}
	return str;
}

bool starts_with(const std::string& str, const std::string& test, const bool case_sensitive) {
	const size_t s1 = str.size();
	const size_t s2 = test.size();
	if (s2 > s1) {
		return false;
	}

	for (size_t i = 0; i < s2; ++i) {
		if (case_sensitive) {
			if (str[i] != test[i]) {
				return false;
			}
		} else {
			if (tolower(str[i]) != tolower(test[i])) {
				return false;
			}
		}
	}

	return true;
}

bool ends_with(const std::string& str, const std::string& test, const bool case_sensitive) {
	const size_t s1 = str.size();
	const size_t s2 = test.size();
	if (s2 > s1) {
		return false;
	}

	const size_t off = s1 - s2;
	for (size_t i = 0; i < s2; ++i) {
		if (case_sensitive) {
			if (str[i + off] != test[i]) {
				return false;
			}
		} else {
			if (tolower(str[i + off]) != tolower(test[i])) {
				return false;
			}
		}
	}

	return true;
}

std::pair<std::string, bool /* changed */> replace_first_or_last(const std::string& str, const std::string& f, const std::string& r, bool first) {
	const size_t pos = (first ? str.find(f) : str.rfind(f));
	if (pos == std::string::npos) {
		return {str, false};
	}

	std::string result = str.substr(0, pos);
	result += r;
	result += str.substr(pos + f.size())
	return {result, true};
}

inline std::string replace_first(const std::string& str, const std::string& f, const std::string& r) {
	return replace_first_or_last(str, f, r, true);
}
inline std::string replace_last(const std::string& str, const std::string& f, const std::string& r) {
	return replace_first_or_last(str, f, r, false);
}

std::string replace_all(std::string str, const std::string& f, const std::string& r) {
	for (;;) {
		const auto pair = replace_first_or_last(str, f, r, true);
		if (!pair.second) {
			break;
		}
		str = pair.first;
	}
	return str;
}
