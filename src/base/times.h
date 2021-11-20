/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#ifndef WL_BASE_TIMES_H
#define WL_BASE_TIMES_H

#include <atomic>
#include <limits>

#include "base/wexception.h"

class FileRead;
class FileWrite;

// The difference of two points in time, in milliseconds gametime.
struct Duration {
	// The default-constructed Duration is the special "invalid" value
	// (semantically an "endless" duration)
	constexpr explicit Duration(uint32_t v = std::numeric_limits<uint32_t>::max()) : value_(v) {
	}

	Duration(const Duration& other) : value_(other.get()) {
	}

	Duration& operator=(const Duration& other) {
		value_ = other.get();
		return *this;
	}

	void operator+=(const Duration& delta) {
		if (!is_valid() || !delta.is_valid()) {
			throw wexception("Attempt to add invalid Durations");
		}
		value_ += delta.get();
	}
	void operator-=(const Duration& delta) {
		if (!is_valid() || !delta.is_valid()) {
			throw wexception("Attempt to subtract invalid Durations");
		}
		if (get() < delta.get()) {
			throw wexception("Duration: Subtraction result would be negative");
		}
		value_ -= delta.get();
	}
	void operator/=(uint32_t d) {
		if (!is_valid()) {
			throw wexception("Attempt to divide invalid Duration");
		}
		if (d == 0) {
			throw wexception("Attempt to divide Duration by zero");
		}
		value_ = value_ / d;
	}

	// Intervals arithmetics
	Duration operator+(const Duration& d) const {
		if (!is_valid() || !d.is_valid()) {
			throw wexception("Attempt to add invalid Durations");
		}
		return Duration(get() + d.get());
	}
	Duration operator-(const Duration& d) const {
		if (!is_valid() || !d.is_valid()) {
			throw wexception("Attempt to subtract invalid Durations");
		}
		if (get() < d.get()) {
			throw wexception("Duration: Subtraction result would be negative");
		}
		return Duration(get() - d.get());
	}
	Duration operator*(uint32_t d) const {
		if (!is_valid()) {
			throw wexception("Attempt to multiply invalid Durations");
		}
		return Duration(get() * d);
	}
	Duration operator/(uint32_t d) const {
		if (!is_valid()) {
			throw wexception("Attempt to divide invalid Duration");
		}
		if (d == 0) {
			throw wexception("Attempt to divide Duration by zero");
		}
		return Duration(get() / d);
	}

	uint32_t get() const {
		return value_.load();
	}

	inline bool operator<(const Duration& m) const {
		return get() < m.get();
	}
	inline bool operator>(const Duration& m) const {
		return get() > m.get();
	}
	inline bool operator<=(const Duration& m) const {
		return get() <= m.get();
	}
	inline bool operator>=(const Duration& m) const {
		return get() >= m.get();
	}
	inline bool operator==(const Duration& m) const {
		return get() == m.get();
	}
	inline bool operator!=(const Duration& m) const {
		return get() != m.get();
	}

	// Special values
	inline bool is_invalid() const {
		return *this == Duration();
	}
	inline bool is_valid() const {
		return !is_invalid();
	}

	// Saveloading
	explicit Duration(FileRead&);
	void save(FileWrite&) const;

private:
	std::atomic<uint32_t> value_;
};

// A time point, in milliseconds gametime.
struct Time {
	// The default-constructed Time is the special "invalid" value
	// (semantically meaning "never")
	constexpr explicit Time(uint32_t v = std::numeric_limits<uint32_t>::max()) : value_(v) {
	}

	Time(const Time& other) : value_(other.get()) {
	}

	Time& operator=(const Time& other) {
		value_ = other.get();
		return *this;
	}

	// Adding/subtracting intervals
	Time operator+(const Duration& delta) const {
		if (!is_valid() || !delta.is_valid()) {
			throw wexception("Attempt to add invalid Time or Duration");
		}
		return Time(get() + delta.get());
	}
	Time operator-(const Duration& delta) const {
		if (!is_valid() || !delta.is_valid()) {
			throw wexception("Attempt to subtract invalid Time or Duration");
		}
		if (get() < delta.get()) {
			throw wexception("Time-Duration-Subtraction result would be negative");
		}
		return Time(get() - delta.get());
	}

	// Obtaining a time difference
	Duration operator-(const Time& t) const {
		if (!is_valid() || !t.is_valid()) {
			throw wexception("Attempt to subtract invalid Time");
		}
		if (get() < t.get()) {
			throw wexception("Time: Subtraction result would be negative");
		}
		return Duration(get() - t.get());
	}

	uint32_t get() const {
		return value_;
	}

	void increment(const Duration& d = Duration(1)) {
		if (!is_valid() || !d.is_valid()) {
			throw wexception("Attempt to increment invalid Time or Duration");
		}
		value_ += d.get();
	}

	inline bool operator<(const Time& m) const {
		return get() < m.get();
	}
	inline bool operator>(const Time& m) const {
		return get() > m.get();
	}
	inline bool operator<=(const Time& m) const {
		return get() <= m.get();
	}
	inline bool operator>=(const Time& m) const {
		return get() >= m.get();
	}
	inline bool operator==(const Time& m) const {
		return get() == m.get();
	}
	inline bool operator!=(const Time& m) const {
		return get() != m.get();
	}

	// Special values
	inline bool is_invalid() const {
		return *this == Time();
	}
	inline bool is_valid() const {
		return !is_invalid();
	}

	// Saveloading
	explicit Time(FileRead&);
	void save(FileWrite&) const;

private:
	std::atomic<uint32_t> value_;
};

#endif  // end of include guard: WL_BASE_TIMES_H
