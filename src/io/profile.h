/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_IO_PROFILE_H
#define WL_IO_PROFILE_H

#include <atomic>
#include <memory>

#include "base/macros.h"
#include "base/vector.h"
#include "io/filesystem/layered_filesystem.h"
// TODO(unknown): as soon as g_fs is not needed anymore,
// include "filesystem.h" instead of layered_filesystem.h.

class Profile;
class FileSystem;

/**
 * Represents one section inside the .ini-style file, basically as a list of
 * key-value pairs.
 *
 * get_string:
 * Returns the value of the first key with the given name. If the key isn't
 * found, def is returned.
 *
 * get_int, get_bool, convert the value string to the desired type.
 *
 * get_safe_*:
 * Like above, but throw an exception if the key doesn't exist.
 *
 * get_next_*:
 * Returns the name of the next key of the given name. You can pass name == 0 to
 * retrieve any unused key. Value::used is used to determine which key is next.
 * The value of the key is stored in the second parameter.
 */
class Section {
public:
	friend class Profile;

	struct Value {
		Value(const std::string& nname, const char* const nval);
		Value(const Value&);
		Value(Value&& other) noexcept;

		// destructor would be empty

		Value& operator=(Value) noexcept;
		Value& operator=(Value&& other) noexcept;

		char const* get_name() const {
			return name_.c_str();
		}

		bool is_used() const;
		void mark_used();

		int32_t get_int() const;
		uint32_t get_natural() const;
		uint32_t get_positive() const;
		bool get_bool() const;
		char const* get_string() const;
		Vector2i get_point() const;

		char const* get_untranslated_string() const {
			return value_.get();
		}
		char* get_untranslated_string() {
			return value_.get();
		}

		void set_string(char const*);

		bool get_translate() const {
			return translate_;
		}
		void set_translate(const bool t) {
			translate_ = t;
		}

		friend void swap(Value& first, Value& second);

	private:
		bool used_;
		bool translate_;
		std::string name_;
		std::unique_ptr<char[]> value_;

		Value() = default;
	};

	using ValueList = std::vector<Value>;

	Section(Profile*, const std::string& name);
	Section(const Section& other) {
		operator=(other);
	}

	Section& operator=(const Section& other) {
		profile_ = other.profile_;
		used_ = other.used_.load();
		section_name_ = other.section_name_;
		values_ = other.values_;
		return *this;
	}

	/// \returns whether a value with the given name exists.
	/// Does not mark the value as used.
	bool has_val(char const* name) const;

	Value* get_val(char const* name);
	Value* get_next_val(char const* name = nullptr);
	uint32_t get_num_values() const {
		return values_.size();
	}

	char const* get_name() const;
	void set_name(const std::string&);

	bool is_used() const;
	void mark_used();

	void check_used() const;

	int32_t get_int(char const* name, int32_t def = 0);
	uint32_t get_natural(char const* name, uint32_t def = 0);
	uint32_t get_positive(char const* name, uint32_t def = 1);
	bool get_bool(char const* name, bool def = false);
	const char* get_string(char const* name, char const* def = nullptr);
	Vector2i get_point(char const* name, Vector2i def = Vector2i::zero());

	int32_t get_safe_int(const char* name);
	uint32_t get_safe_natural(char const* name);
	uint32_t get_safe_positive(char const* name);
	bool get_safe_bool(const char* name);
	const char* get_safe_untranslated_string(const char* name);
	const char* get_safe_string(const char* name);
	const char* get_safe_string(const std::string& name);

	char const* get_next_bool(char const* name, bool* value);

	void set_int(char const* name, int32_t value);
	void set_natural(char const* name, uint32_t value);
	void set_bool(char const* const name, bool const value) {
		set_string(name, value ? "true" : "false");
	}
	void set_string(char const* name, char const* string);
	void set_string_duplicate(char const* name, char const* string);
	void set_string(char const* const name, const std::string& string) {
		set_string(name, string.c_str());
	}
	void set_string_duplicate(char const* const name, const std::string& string) {
		set_string_duplicate(name, string.c_str());
	}

	void set_translated_string(char const* name, char const* string);
	void set_translated_string_duplicate(char const* name, char const* string);
	void set_translated_string(char const* const name, const std::string& string) {
		set_translated_string(name, string.c_str());
	}
	void set_translated_string_duplicate(char const* const name, const std::string& string) {
		set_translated_string_duplicate(name, string.c_str());
	}

	/// If a Value with this name already exists, update it with the given
	/// value. Otherwise create a new Value with the given name and value.
	Value& create_val(char const* name, char const* value);

	/// Create a new Value with the given name and value.
	Value& create_val_duplicate(char const* name, char const* value);

private:
	Profile* profile_;
	std::atomic_bool used_;
	std::string section_name_;
	ValueList values_;
};

/**
 * Parses an .ini-style file into sections and key-value pairs.
 * The destructor prints out warnings if a section or value hasn't been used.
 *
 * get_section:
 * Returns the first section of the given name (or 0 if none exist).
 *
 * get_safe_section:
 * Like get_section, but throws an exception if the section doesn't exist.
 *
 * get_next_section:
 * Returns the next unused section of the given name, or 0 if all sections
 * have been used. name can be 0 to retrieve any remaining sections.
 */
class Profile {
public:
	enum { err_ignore = 0, err_log, err_throw };

	explicit Profile(int32_t error_level = err_throw);
	explicit Profile(char const* filename,
	                 char const* global_section = nullptr,
	                 int32_t error_level = err_throw);
	explicit Profile(char const* filename,
	                 char const* global_section,
	                 const std::string& textdomain,
	                 int32_t error_level = err_throw);

	void error(char const*, ...) const __attribute__((format(printf, 2, 3)));
	void check_used() const;

	void read(const char* const filename,
	          const char* const global_section = nullptr,
	          FileSystem& = *g_fs);
	void write(const char* const filename,
	           bool used_only = true,
	           FileSystem& = *g_fs,
	           const char* const comment = nullptr);

	Section* get_section(const std::string& name);
	Section& get_safe_section(const std::string& name);
	Section& pull_section(char const* name);
	Section* get_next_section(char const* name = nullptr);

	/// If a section with the given name already exists, return a reference to
	/// it. Otherwise create a new section with the given name and return a
	/// reference to it.
	Section& create_section(char const* name);

	/// Create a new section with the given name and return a reference to it.
	Section& create_section_duplicate(char const* name);

private:
	using SectionList = std::vector<Section>;

	std::string filename_;
	SectionList sections_;
	int32_t error_level_;

	DISALLOW_COPY_AND_ASSIGN(Profile);
};

#endif  // end of include guard: WL_IO_PROFILE_H
