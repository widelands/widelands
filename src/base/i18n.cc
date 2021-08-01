/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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

#include "base/i18n.h"

#ifdef __FreeBSD__
#include <clocale>
#endif

#include <cstdlib>
#include <map>

#include <boost/format.hpp>

#include "base/log.h"
#include "config.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"

#ifdef __APPLE__
#if LIBINTL_VERSION >= 0x001201
// for older libintl versions, setlocale is just fine
#define SETLOCALE libintl_setlocale
#endif  // LIBINTL_VERSION
#else   // __APPLE__
#if defined _WIN32
#define SETLOCALE setlocale
#else
#define SETLOCALE std::setlocale
#endif
#endif

extern int _nl_msg_cat_cntr;

namespace i18n {

/// A stack of textdomains. On entering a new textdomain, the old one gets
/// pushed on the stack. On leaving the domain again it is popped back.
/// \see grab_texdomain()
namespace {

std::vector<std::pair<std::string, std::string>> textdomains;

std::string env_locale;
std::string locale;
std::string localedir;
std::string homedir;

char* (*gettext_)(const char*) = gettext;

char* verbose_gettext(const char* s) {
	log_dbg("gettext: %s\n", s);
	return gettext(s);
}

}  // namespace

void enable_verbose_i18n() {
	gettext_ = verbose_gettext;
}

/**
 * Translate a string with gettext
 */
// TODO(unknown): Implement a workaround if gettext was not found
char const* translate(char const* const str) {
	return gettext_(str);
}
char const* translate(const std::string& str) {
	return gettext_(str.c_str());
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

GenericTextdomain::GenericTextdomain() : lock_(MutexLock::ID::kI18N) {
}
GenericTextdomain::~GenericTextdomain() {
	release_textdomain();
}
Textdomain::Textdomain(const std::string& name) : GenericTextdomain() {
	grab_textdomain(name, get_localedir().c_str());
}
AddOnTextdomain::AddOnTextdomain(std::string addon, const int i18n_version) : GenericTextdomain() {
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
void grab_textdomain(const std::string& domain, const char* ldir) {
	char const* const dom = domain.c_str();

	bindtextdomain(dom, ldir);
	bind_textdomain_codeset(dom, "UTF-8");
	// log("textdomain %s @ %s\n", dom, ldir);
	textdomain(dom);
	textdomains.push_back(std::make_pair(dom, ldir));
}

/**
 * See grab_textdomain()
 */
void release_textdomain() {
	if (textdomains.empty()) {
		log_err("trying to pop textdomain from empty stack");
		return;
	}
	textdomains.pop_back();

	// Don't try to get the previous TD when the very first one ('widelands')
	// just got dropped
	if (!textdomains.empty()) {
		char const* const domain = textdomains.back().first.c_str();

		bind_textdomain_codeset(domain, "UTF-8");
		bindtextdomain(domain, textdomains.back().second.c_str());
		textdomain(domain);
	}
}

/**
 * Initialize locale to English.
 * Save system language (for later selection).
 * Code inspired by wesnoth.org
 */
void init_locale() {
	env_locale = std::string();
#ifdef _WIN32
	locale = "English";
	SETLOCALE(LC_ALL, "English");
#else
	// first, save environment variable
	char* lang;
	lang = getenv("LANG");
	if (lang != nullptr) {
		env_locale = lang;
	}
	if (env_locale.empty()) {
		lang = getenv("LANGUAGE");
		if (lang != nullptr) {
			env_locale = lang;
		} else {  // Finall fallback in case we cannot find out anything (#1784495)
			env_locale = "en";
		}
	}
	locale = "C";
	SETLOCALE(LC_ALL, "C");
	SETLOCALE(LC_MESSAGES, "");
#endif
}

static std::string canonical_addon_locale_dir;
const std::string& get_addon_locale_dir() {
	if (canonical_addon_locale_dir.empty()) {
		canonical_addon_locale_dir = g_fs->canonicalize_name(homedir + "/" + kAddOnLocaleDir);
		assert(!canonical_addon_locale_dir.empty());
	}
	return canonical_addon_locale_dir;
}

/**
 * Set the locale to the given string.
 * Code inspired by wesnoth.org
 */
void set_locale(const std::string& name) {
	const std::map<std::string, std::string> kAlternatives = {
	   {"ar", "ar,ar_AR,ar_AE,ar_BH,ar_DZ,ar_EG,ar_IN,ar_IQ,ar_JO,ar_KW,ar_LB,ar_LY,ar_MA,ar_OM,ar_"
	          "QA,ar_SA,ar_SD,ar_SY,ar_TN,ar_YE"},
	   {"ast", "ast,ast_ES"},
	   {"ca", "ca,ca_ES,ca_ES@valencia,ca_FR,ca_IT"},
	   {"cs", "cs,cs_CZ"},
	   {"da", "da,da_DK"},
	   {"de", "de,de_DE,de_AT,de_CH,de_LI,de_LU,de_BE"},
	   {"el", "el,el_GR,el_CY"},
	   {"en", "en,en_US,en_GB,en_AU,en_CA,en_AG,en_BW,en_DK,en_HK,en_IE,en_IN,en_NG,en_NZ,en_PH,en_"
	          "SG,en_ZA,en_ZW"},
	   {"en_AU", "en_AU,en,en_US,en_GB"},
	   {"en_CA", "en_CA,en,en_US,en_GB"},
	   {"en_GB", "en_GB,en,en_US"},
	   {"eo", "eo,eo_XX"},
	   {"es", "es,es_ES,es_MX,es_US"},
	   {"et", "et,et_EE"},
	   {"eu", "eu,eu_ES,eu_FR"},
	   {"fa", "fa,fa_IR"},
	   {"fi", "fi,fi_FI"},
	   {"fr", "fr,fr_FR,fr_CH,fr_BE,fr_CA,fr_LU"},
	   {"gd", "gd,gd_GB,gd_CA"},
	   {"gl", "ga,gl_ES"},
	   {"he", "he,he_IL"},
	   {"hr", "hr,hr_HR,hr_RS,hr_BA,hr_ME,hr_HU"},
	   {"hu", "hu,hu_HU"},
	   {"ia", "ia"},
	   {"id", "id,id_ID"},
	   {"it", "it,it_IT,it_CH"},
	   {"ja", "ja,ja_JP"},
	   {"jv", "jv,jv_ID,jv_MY,jv_SR,jv_NC"},
	   {"ka", "ka,ka_GE"},
	   {"ko", "ko,ko_KR"},
	   {"la", "la,la_AU,la_VA"},
	   {"mr", "mr,mr_IN"},
	   {"ms", "ms,ms_MY"},
	   {"my", "my,my_MM"},
	   {"nb", "nb,nb_NO"},
	   {"nl", "nl,nl_NL,nl_BE,nl_AW"},
	   {"nn", "nn,nn_NO"},
	   {"oc", "oc,oc_FR"},
	   {"pl", "pl,pl_PL"},
	   {"pt", "pt,pt_PT,pt_BR"},
	   {"pt_BR", "pt_BR,pt,pt_PT"},
	   {"ru", "ru,ru_RU,ru_UA"},
	   {"si", "si,si_LK"},
	   {"sk", "sk,sk_SK"},
	   {"sl", "sl,sl_SI"},
	   {"sr", "sr,sr_RS,sr_ME"},
	   {"sv", "sv,sv_FI,sv_SE"},
	   {"tr", "tr,tr_TR,tr_CY"},
	   {"uk", "uk,uk_UA"},
	   {"vi", "vi,vi_VN"},
	   {"zh", "zh,zh_CN,zh_TW,zh_HK,zh_MO"},
	   {"zh_CN", "zh_CN,zh,zh_TW,zh_HK,zh_MO"},
	   {"zh_TW", "zh_TW,zh_HK,zh_MO,zh,zh_CN"},
	};

	std::string lang(name);

	log_info("selected language: %s\n", lang.empty() ? "(system language)" : lang.c_str());

#ifndef _WIN32
#ifndef __AMIGAOS4__
#ifndef __APPLE__
	unsetenv("LANGUAGE");  // avoid problems with this variable
#endif
#endif
#endif

	std::string alt_str;
	if (lang.empty()) {
		// reload system language, if selected
		lang = env_locale;
		alt_str = env_locale;
	} else {
		alt_str = lang;
		// otherwise, try alternatives.
		if (kAlternatives.count(lang)) {
			alt_str = kAlternatives.at(lang);
		}
	}
	alt_str += ",";

// Somehow setlocale doesn't behave same on
// some systems.
#ifdef __BEOS__
	setenv("LANG", lang.c_str(), 1);
	setenv("LC_ALL", lang.c_str(), 1);
	locale = lang;
#endif
#ifdef __APPLE__
	setenv("LANGUAGE", lang.c_str(), 1);
	setenv("LANG", lang.c_str(), 1);
	setenv("LC_ALL", lang.c_str(), 1);
	locale = lang;
#endif
#ifdef _WIN32
	_putenv_s("LANG", lang.c_str());
	locale = lang;
#endif

#ifdef __linux__
	char* res = nullptr;
	char const* encodings[] = {"", ".utf-8", "@euro", ".UTF-8"};
	std::size_t found = alt_str.find(',', 0);
	bool leave_while = false;
	// try every possible combination of alternative and encoding
	while (found != std::string::npos) {
		std::string base_locale = alt_str.substr(0, int(found));
		alt_str = alt_str.erase(0, int(found) + 1);

		for (char const* encoding : encodings) {
			std::string try_locale = base_locale + encoding;
			res = SETLOCALE(LC_MESSAGES, try_locale.c_str());
			if (res) {
				locale = try_locale;
				log_info("using locale %s\n", try_locale.c_str());
				leave_while = true;
				break;
			} else {
				// log("locale is not working: %s\n", try_locale.c_str());
			}
		}
		if (leave_while) {
			break;
		}

		found = alt_str.find(',', 0);
	}
	if (leave_while) {
		setenv("LC_ALL", locale.c_str(), 1);
		setenv("LANG", locale.c_str(), 1);
		setenv("LANGUAGE", locale.c_str(), 1);
	} else {
		log_warn("No corresponding locale found\n");
		log_warn(" - Set LANGUAGE, LANG and LC_ALL to '%s'\n", lang.c_str());

		setenv("LANGUAGE", lang.c_str(), 1);
		setenv("LANG", lang.c_str(), 1);
		setenv("LC_ALL", lang.c_str(), 1);

		try {
			SETLOCALE(LC_MESSAGES, "en_US.utf8");  // set locale according to the env. variables
			                                       // --> see  $ man 3 setlocale
			log_warn(" - Set system locale to 'en_US.utf8' to make '%s' accessible to libintl\n",
			         lang.c_str());
		} catch (std::exception&) {
			SETLOCALE(LC_MESSAGES, "");  // set locale according to the env. variables
			                             // --> see  $ man 3 setlocale
		}
		// assume that it worked
		// maybe, do another check with the return value (?)
		locale = lang;
	}

	/* Finally make changes known.  */
	++_nl_msg_cat_cntr;
#endif

	SETLOCALE(LC_ALL, "");  //  call to libintl

	if (!textdomains.empty()) {
		char const* const domain = textdomains.back().first.c_str();

		bind_textdomain_codeset(domain, "UTF-8");
		bindtextdomain(domain, textdomains.back().second.c_str());
		textdomain(domain);
	}
}

const std::string& get_locale() {
	return locale;
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
				result = (boost::format(_("%1$s & %2$s")) % result % (*it)).str();
			} else if (listtype == ConcatenateWith::OR) {
				/** TRANSLATORS: Join the last 2 items on a list with "or". */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = (boost::format(_("%1$s or %2$s")) % result % (*it)).str();
			} else if (listtype == ConcatenateWith::COMMA) {
				/** TRANSLATORS: Join the last 2 items on a list with a comma. */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = (boost::format(_("%1$s, %2$s")) % result % (*it)).str();
			} else {
				/** TRANSLATORS: Concatenate the last 2 items on a list. */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = (boost::format(_("%1$s and %2$s")) % result % (*it)).str();
			}
		} else {
			/** TRANSLATORS: Concatenate 2 items at in the middle of a list. */
			/** TRANSLATORS: RTL languages might want to change the word order here. */
			result = (boost::format(_("%1$s, %2$s")) % result % (*it)).str();
		}
	}
	return result;
}

std::string join_sentences(const std::string& sentence1, const std::string& sentence2) {
	i18n::Textdomain td("widelands");
	/** TRANSLATORS: Put 2 sentences one after the other. Languages using Chinese script probably
	 * want to lose the blank space here. */
	return (boost::format(pgettext("sentence_separator", "%1% %2%")) % sentence1 % sentence2).str();
}

}  // namespace i18n
