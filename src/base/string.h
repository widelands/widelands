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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_BASE_STRING_H
#define WL_BASE_STRING_H

#include <algorithm>
#include <functional>
#include <set>
#include <string>
#include <vector>

/** Split a string into substrings at all occurrences of any of the given delimiters. */
void split(std::vector<std::string>& result, const std::string&, const std::set<char>&);

/** Convert a string to lowercase. */
inline std::string to_lower(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), tolower);
	return result;
}

/** Check whether two strings are equal regardless of case. */
inline bool iequals(const std::string& a, const std::string& b) {
	return to_lower(a) == to_lower(b);
}

/**
 * Check whether `str` starts with / ends with / contains `test`.
 * Can be case-sensitive or -insensitive depending on the last argument, default is sensitive.
 */
inline bool
starts_with(const std::string& str, const std::string& test, bool case_sensitive = true) {
	const size_t s1 = str.size();
	const size_t s2 = test.size();
	return s1 >= s2 && 0 == (case_sensitive ? str.compare(0, s2, test) :
                                             to_lower(str).compare(0, s2, to_lower(test)));
}
inline bool ends_with(const std::string& str, const std::string& test, bool case_sensitive = true) {
	const size_t s1 = str.size();
	const size_t s2 = test.size();
	return s1 >= s2 && 0 == (case_sensitive ? str.compare(s1 - s2, s2, test) :
                                             to_lower(str).compare(s1 - s2, s2, to_lower(test)));
}
inline bool contains(const std::string& str, const std::string& test, bool case_sensitive = true) {
	return (case_sensitive ? str.find(test) : to_lower(str).find(to_lower(test))) !=
	       std::string::npos;
}

/** Remove leading and/or trailing whitespace from a string. */
void trim(std::string&, bool remove_leading = true, bool remove_trailing = true);

/** Helper function for the replace_* functions. */
bool replace_first_or_last(std::string&, const std::string&, const std::string&, bool);

/** Replace all / the first / the last occurrence(s) of `f` in `str` with `r`. */
void replace_all(std::string& str, const std::string& f, const std::string& r);
inline void replace_first(std::string& str, const std::string& f, const std::string& r) {
	replace_first_or_last(str, f, r, true);
}
inline void replace_last(std::string& str, const std::string& f, const std::string& r) {
	replace_first_or_last(str, f, r, false);
}

/** Concatenate all strings in `words` with the given `separator` between them. */
template <typename Container>
std::string join(const Container& words, const std::string& separator) {
	if (words.empty()) {
		return std::string();
	}
	auto it = words.begin();
	std::string str = *it;
	for (; it != words.end();) {
		str += separator;
		str += *(++it);
	}
	return str;
}

/** Convert various types to string. Useful in templates where the parameter type is not fixed. */
inline const std::string& as_string(const std::string& str) {
	return str;
}
inline std::string as_string(const char* str) {
	return str;
}
inline std::string as_string(const int8_t t) {
	return std::to_string(static_cast<int>(t));
}
inline std::string as_string(const int16_t t) {
	return std::to_string(static_cast<int>(t));
}
inline std::string as_string(const int32_t t) {
	return std::to_string(static_cast<int64_t>(t));
}
inline std::string as_string(const int64_t t) {
	return std::to_string(t);
}
inline std::string as_string(const uint8_t t) {
	return std::to_string(static_cast<unsigned>(t));
}
inline std::string as_string(const uint16_t t) {
	return std::to_string(static_cast<unsigned>(t));
}
inline std::string as_string(const uint32_t t) {
	return std::to_string(static_cast<uint64_t>(t));
}
inline std::string as_string(const uint64_t t) {
	return std::to_string(t);
}
inline std::string as_string(char c) {
	return {c, '\0'};
}

#endif  // end of include guard: WL_BASE_STRING_H
