/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#define BASE_I18N_CC

#include "base/i18n.h"

#ifdef __FreeBSD__
#include <clocale>
#endif

#include <cstdlib>
#include <map>
#include <memory>

#include "base/log.h"
#include "base/string.h"
#include "config.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "third_party/tinygettext/include/tinygettext/log.hpp"
#include "third_party/tinygettext/include/tinygettext/tinygettext.hpp"

namespace i18n {

/// A stack of textdomains. On entering a new textdomain, the old one gets
/// pushed on the stack. On leaving the domain again it is popped back.
/// \see grab_texdomain()
namespace {

struct TextdomainStackEntry {
	explicit TextdomainStackEntry(const std::string& dir) {
		if (g_fs->is_directory(dir)) {
			dictionary_manager.add_directory(dir);
		} else {
			log_warn("Textdomain directory %s does not exist", dir.c_str());
		}
	}

	const std::string& translate_ctxt_plural(const std::string& ctxt, const std::string& sg, const std::string& pl, int n) {
		cached_return_values.push_back(dictionary().translate_ctxt_plural(ctxt, sg, pl, n));
		return cached_return_values.back();
	}
	const std::string& translate_ctxt(const std::string& ctxt, const std::string& msg) {
		cached_return_values.push_back(dictionary().translate_ctxt(ctxt, msg));
		return cached_return_values.back();
	}
	const std::string& translate_plural(const std::string& sg, const std::string& pl, int n) {
		cached_return_values.push_back(dictionary().translate_plural(sg, pl, n));
		return cached_return_values.back();
	}
	const std::string& translate(const std::string& msg) {
		cached_return_values.push_back(dictionary().translate(msg));
		return cached_return_values.back();
	}

private:
	tinygettext::DictionaryManager dictionary_manager;

	// To prevent translations from going out of scope before use in complex string assemblies.
	std::vector<std::string> cached_return_values;

	tinygettext::Dictionary& dictionary() {
		return dictionary_manager.get_dictionary(tinygettext::Language::from_env(get_locale_or_default()));
	}
};

std::vector<std::unique_ptr<TextdomainStackEntry>> textdomains;

std::string env_locale;
std::string locale;
std::string localedir;
std::string homedir;

void log_i18n_off(const char* /*unused*/, const char* /*unused*/) {
}
void log_i18n_on(const char* type, const char* msg) {
	log_dbg("%s: %s", type, msg);
}

}  // namespace

static void (*log_i18n_if_desired_)(const char*, const char*) = log_i18n_off;

void enable_verbose_i18n() {
	log_i18n_if_desired_ = log_i18n_on;
}

static void tinygettext_log_err(const std::string& msg) {
	log_err("%s", msg.c_str());
}
static void tinygettext_log_warn(const std::string& msg) {
	log_warn("%s", msg.c_str());
}
static void tinygettext_log_info(const std::string& msg) {
	verb_log_info("%s", msg.c_str());
}

/**
 * Translate a string.
 */
const char* translate(const char* const str) {
	return translate(std::string(str)).c_str();
}
const std::string& translate(const std::string& str) {
	log_i18n_if_desired_("gettext", str.c_str());
	if (textdomains.empty()) {
		log_err("Call to translate with empty textdomain stack");
		return str;
	}
	return textdomains.back()->translate(str);
}

const char* pgettext_wrapper(const char* msgctxt, const char* msgid) {
	return pgettext_wrapper(std::string(msgctxt), std::string(msgid)).c_str();
}
const std::string& pgettext_wrapper(const std::string& msgctxt, const std::string& msgid) {
	log_i18n_if_desired_("pgettext", msgid.c_str());
	if (textdomains.empty()) {
		log_err("Call to pgettext with empty textdomain stack");
		return msgid;
	}
	return textdomains.back()->translate_ctxt(msgctxt, msgid);
}

const char* ngettext_wrapper(const char* singular, const char* plural, const int n) {
	return ngettext_wrapper(std::string(singular), std::string(plural), n).c_str();
}
const std::string& ngettext_wrapper(const std::string& singular, const std::string& plural, const int n) {
	log_i18n_if_desired_("ngettext", singular.c_str());
	if (textdomains.empty()) {
		log_err("Call to ngettext with empty textdomain stack");
		return n == 1 ? singular : plural;
	}
	return textdomains.back()->translate_plural(singular, plural, n);
}

const char* npgettext_wrapper(const char* msgctxt, const char* singular, const char* plural, int n) {
	return npgettext_wrapper(std::string(msgctxt), std::string(singular), std::string(plural), n).c_str();
}
const std::string& npgettext_wrapper(const std::string& msgctxt, const std::string& singular, const std::string& plural, int n) {
	log_i18n_if_desired_("npgettext", singular.c_str());
	if (textdomains.empty()) {
		log_err("Call to npgettext with empty textdomain stack");
		return n == 1 ? singular : plural;
	}
	return textdomains.back()->translate_ctxt_plural(msgctxt, singular, plural, n);
}

/**
 * Set the localedir. This should usually only be done once
 */
void set_localedir(const std::string& dname) {
	localedir = dname;
}

const std::string& get_localedir() {
	return localedir;
}

void set_homedir(const std::string& dname) {
	homedir = dname;
}

const std::string& get_homedir() {
	return homedir;
}

static std::string canonical_addon_locale_dir;
const std::string& get_addon_locale_dir() {
	if (canonical_addon_locale_dir.empty()) {
		canonical_addon_locale_dir = g_fs->canonicalize_name(homedir + "/" + kAddOnLocaleDir);
		assert(!canonical_addon_locale_dir.empty());
	}
	return canonical_addon_locale_dir;
}

GenericTextdomain::GenericTextdomain() : lock_(MutexLock::ID::kI18N) {
}
GenericTextdomain::~GenericTextdomain() {
	release_textdomain();
}
Textdomain::Textdomain(const std::string& name) {
	grab_textdomain(name, get_localedir().c_str());
}
AddOnTextdomain::AddOnTextdomain(std::string addon, const int i18n_version) {
	addon += '.';
	addon += std::to_string(i18n_version);
	grab_textdomain(addon, get_addon_locale_dir().c_str());
}

/**
 * Grab a given TextDomain. If a new one is grabbed, it is pushed on the stack.
 * On release, it is dropped and the previous one is re-grabbed instead.
 *
 * So when a tribe loads, it grabs its textdomain, loads all data and releases
 * it -> we're back in widelands domain. Negative: We can't translate error
 * messages. Who cares?
 */
void grab_textdomain(const std::string& domain, const std::string& ldir) {
	log_i18n_if_desired_("textdomain", (domain + " @ " + ldir).c_str());
	textdomains.emplace_back(new TextdomainStackEntry(ldir + "/" + domain));
}

/**
 * See grab_textdomain()
 */
void release_textdomain() {
	if (textdomains.empty()) {
		log_err("Trying to pop textdomain from empty stack");
		return;
	}
	textdomains.pop_back();
}

void set_locale(const std::string& name) {
	locale = name;
}
const std::string& get_locale() {
	return locale;
}
const std::string& get_locale_or_default() {
	return locale.empty() ? env_locale : locale;
}

void init_locale() {
	tinygettext::Log::set_log_error_callback(tinygettext_log_err);
	tinygettext::Log::set_log_warning_callback(tinygettext_log_warn);
	tinygettext::Log::set_log_info_callback(tinygettext_log_info);

	locale = std::string();
	env_locale = "en";
	for (const auto& var : {"LANG", "LANGUAGE"}) {
		const char *environment_variable = getenv(var);
		if (environment_variable != nullptr && environment_variable[0] != '\0') {
			env_locale = environment_variable;
			break;
		}
	}
}

std::string localize_list(const std::vector<std::string>& items, ConcatenateWith listtype) {
	i18n::Textdomain td("widelands");
	std::string result;
	for (std::vector<std::string>::const_iterator it = items.begin(); it != items.end(); ++it) {
		if (it == items.begin()) {
			result = *it;
		} else if (it == --items.end()) {
			if (listtype == ConcatenateWith::AMPERSAND) {
				/** TRANSLATORS: Concatenate the last 2 items on a list. */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = format(_("%1$s & %2$s"), result, (*it));
			} else if (listtype == ConcatenateWith::OR) {
				/** TRANSLATORS: Join the last 2 items on a list with "or". */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = format(_("%1$s or %2$s"), result, (*it));
			} else if (listtype == ConcatenateWith::COMMA) {
				/** TRANSLATORS: Join the last 2 items on a list with a comma. */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = format(_("%1$s, %2$s"), result, (*it));
			} else {
				/** TRANSLATORS: Concatenate the last 2 items on a list. */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = format(_("%1$s and %2$s"), result, (*it));
			}
		} else {
			/** TRANSLATORS: Concatenate 2 items at in the middle of a list. */
			/** TRANSLATORS: RTL languages might want to change the word order here. */
			result = format(_("%1$s, %2$s"), result, (*it));
		}
	}
	return result;
}

std::string join_sentences(const std::string& sentence1, const std::string& sentence2) {
	i18n::Textdomain td("widelands");
	/** TRANSLATORS: Put 2 sentences one after the other. Languages using Chinese script probably
	 * want to lose the blank space here. */
	return format(pgettext_wrapper("sentence_separator", "%1% %2%"), sentence1, sentence2);
}

bool is_translation_of(const std::string& input,
                       const std::string& base,
                       const std::string& textdomain_name) {
	if (input == base) {
		return true;
	}
	if (textdomain_name.empty()) {
		return input == translate(base);
	}

	Textdomain td(textdomain_name);
	return input == translate(base);
}

}  // namespace i18n
