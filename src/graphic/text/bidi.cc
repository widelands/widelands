/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#include "graphic/text/bidi.h"

#include <map>
#include <string>

#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

#include "base/log.h"

namespace {
// TODO(GunChleoc): Have a look at the ICU API to see which helper functions can be gained from there.
// NOCOM(GunChleoc): Turn this into a proper class

// Need to mirror () etc. for LTR languages, so we're sticking them in a map.
const std::map<UChar, UChar> kSymmetricChars = {
	{0x0028, 0x0029}, // ()
	{0x0029, 0x0028}, // )(
	{0x003C, 0x003E}, // <>
	{0x003E, 0x003C}, // ><
	{0x005B, 0x005D}, // []
	{0x005D, 0x005B}, // ][
	{0x007B, 0x007D}, // {}
	{0x007D, 0x007B}, // }{
	{0x201C, 0x201D}, // “”
	{0x201D, 0x201C}, // ”“
	{0x2018, 0x2019}, // ‘’
	{0x2019, 0x2018}, // ’‘
};

bool is_symmetric_char(UChar c) {
	return kSymmetricChars.count(c) == 1;
}

UChar mirror_symmetric_char(UChar c) {
	if (kSymmetricChars.count(c) == 1) {
		c = kSymmetricChars.at(c);
	}
	return c;
}

bool is_numeric_char(UChar c) {
	return 0x0030 <= c && c <= 0x0039;  // 0-9
}

bool is_latin_char(UChar c) {
	return (0x0061 <= c && c <= 0x007a) ||  // a-z
			 (0x0041 <= c && c <= 0x005A);    // A-Z
}

bool is_punctuation_char(UChar c) {
	return c == 0x0020 ||  // blank space
			 c == 0x0021 ||  // !
			 c == 0x002C ||  // ,
			 c == 0x002D ||  // - // NOCOM " - " becomes "-  "
			 c == 0x002E ||  // .
			 c == 0x002F ||  // /
			 c == 0x003A ||  // :
			 c == 0x003B ||  // ;
			 c == 0x003F ||  // ?
			 c == 0x005C ||  // backslash
			 (c >= 0x2000 && c <= 0x206F); // en-dash, em-dash etc.
}


// TODO(GunChleoc): Presentation forms A on demand
// http://unicode-table.com/en/blocks/arabic-presentation-forms-a/
const std::map<UChar, UChar> kArabicFinalChars = {
	{0x0622, 0xFE82}, // ʾalif maddah
	{0x0623, 0xFE84}, // ʾalif with hamza above
	{0x0624, 0xFE86}, // wāw with hamza above
	{0x0625, 0xFE88}, // ʾalif with hamza below
	{0x0626, 0xFE8A}, // yāʾ with hamza above
	{0x0627, 0xFE8E}, // ʾalif
	{0x0628, 0xFE90}, // bāʾ
	{0x0629, 0xFE94}, // tāʾ marbūṭah
	{0x062A, 0xFE96}, // tāʾ
	{0x062B, 0xFE9A}, // ṯāʾ
	{0x062C, 0xFE9E}, // ǧīm
	{0x062D, 0xFEA2}, // ḥāʾ
	{0x062E, 0xFEA6}, // ḫāʾ
	{0x062F, 0xFEAA}, // dāl
	{0x0630, 0xFEAC}, // ḏāl
	{0x0631, 0xFEAE}, // rāʾ
	{0x0632, 0xFEB0}, // zayn/zāy
	{0x0633, 0xFEB2}, // sīn
	{0x0634, 0xFEB6}, // šīn
	{0x0635, 0xFEBA}, // ṣād
	{0x0636, 0xFEBE}, // ḍād
	{0x0637, 0xFEC2}, // ṭāʾ
	{0x0638, 0xFEC6}, // ẓāʾ
	{0x0639, 0xFECA}, // ʿayn
	{0x063A, 0xFECE}, // ġayn
	/*
	{0x063B, 0x}, // Keheh with Two Dots Above
	{0x063C, 0x}, // Keheh with Three Dots Below
	{0x063D, 0x}, // Farsi Yeh with Inverted V
	{0x063E, 0x}, // Farsi Yeh with Two Dots Above
	{0x063F, 0x}, // Farsi Yeh with Three Dots Above
		*/
	{0x0641, 0xFED2}, // fāʾ
	{0x0642, 0xFED6}, // qāf
	{0x0643, 0xFEDA}, // kāf
	{0x0644, 0xFEDE}, // lām
	{0x0645, 0xFEE2}, // mīm
	{0x0646, 0xFEE6}, // nūn
	{0x0647, 0xFEEA}, // hāʾ
	{0x0648, 0xFEEE}, // wāw
	{0x0649, 0xFEF0}, // ʾalif maqṣūrah
	{0x064A, 0xFEF2}, // yāʾ
	{0xFD3D, 0xFD3C}, // Ligature Alef with Fathatan
	{0xFEF5, 0xFEF6}, // lām ʾalif maddah
	{0xFEF7, 0xFEF8}, // lām ʾalif hamza above
	{0xFEF9, 0xFEFA}, // lām ʾalif hamza below
	{0xFEFB, 0xFEFC}, // lām ʾalif
};

const std::map<UChar, UChar> kArabicInitialChars = {
	{0x0626, 0xFE8B}, // yāʾ with hamza above
	{0x0628, 0xFE91}, // bāʾ
	{0x062A, 0xFE97}, // tāʾ
	{0x062B, 0xFE9B}, // ṯāʾ
	{0x062C, 0xFE9F}, // ǧīm
	{0x062D, 0xFEA3}, // ḥāʾ
	{0x062E, 0xFEA7}, // ḫāʾ
	{0x0633, 0xFEB3}, // sīn
	{0x0634, 0xFEB7}, // šīn
	{0x0635, 0xFEBB}, // ṣād
	{0x0636, 0xFEBF}, // ḍād
	{0x0637, 0xFEC3}, // ṭāʾ
	{0x0638, 0xFEC7}, // ẓāʾ
	{0x0639, 0xFECB}, // ʿayn
	{0x063A, 0xFECF}, // ġayn
	{0x0641, 0xFED3}, // fāʾ
	{0x0642, 0xFED7}, // qāf
	{0x0643, 0xFEDB}, // kāf
	{0x0644, 0xFEDF}, // lām
	{0x0645, 0xFEE3}, // mīm
	{0x0646, 0xFEE7}, // nūn
	{0x0647, 0xFEEB}, // hāʾ
	{0x064A, 0xFEF3}, // yāʾ
};

const std::map<UChar, UChar> kArabicMedialChars = {
	{0x0626, 0xFE8C}, // yāʾ with hamza above
	{0x0628, 0xFE92}, // bāʾ
	{0x062A, 0xFE98}, // tāʾ
	{0x062B, 0xFE9C}, // ṯāʾ
	{0x062C, 0xFEA0}, // ǧīm
	{0x062D, 0xFEA4}, // ḥāʾ
	{0x062E, 0xFEA8}, // ḫāʾ
	{0x0633, 0xFEB4}, // sīn
	{0x0634, 0xFEB8}, // šīn
	{0x0635, 0xFEBC}, // ṣād
	{0x0636, 0xFEC0}, // ḍād
	{0x0637, 0xFEC4}, // ṭāʾ
	{0x0638, 0xFEC8}, // ẓāʾ
	{0x0639, 0xFECC}, // ʿayn
	{0x063A, 0xFED0}, // ġayn
	{0x0641, 0xFED4}, // fāʾ
	{0x0642, 0xFED8}, // qāf
	{0x0643, 0xFEDB}, // kāf
	{0x0644, 0xFEE0}, // lām
	{0x0645, 0xFEE4}, // mīm
	{0x0646, 0xFEE8}, // nūn
	{0x0647, 0xFEEC}, // hāʾ
	{0x064A, 0xFEF4}, // yāʾ
};


// Special ligature forms combine 2 letters. lām-alif ligature is mandatory.
// Diacritics also form ligatures.
const std::map<std::pair<UChar, UChar>, UChar> kArabicLigatures = {
	{{0x0644, 0x0622}, 0xFEF5}, // lām ʾalif maddah
	{{0x0644, 0x0623}, 0xFEF7}, // lām ʾalif hamza above
	{{0x0644, 0x0625}, 0xFEF9}, // lām ʾalif hamza below
	{{0x0644, 0x0627}, 0xFEFB}, // lām ʾalif
	{{0x0627, 0x064B}, 0xFD3D}, // Ligature Alef with Fathatan Isolated Form

	// Basing these off the isolated forms; I don't know what will happen with canonical forms here
	{{0xFE7C, 0xFE72}, 0xFC5E}, // Shadda with Dammatan Isolated Form
	{{0xFE7C, 0xFE74}, 0xFC5F}, // Shadda with Kasratan Isolated Form
	{{0xFE7C, 0x064D}, 0xFE76}, // Shadda with Fatha Isolated Form
	{{0xFE7C, 0xFE78}, 0xFC61}, // Shadda with Damma Isolated Form
	{{0xFE7C, 0xFE7A}, 0xFC62}, // Shadda with Kasra Isolated Form
	{{0xFE7C, 0x0670}, 0xFC63}, // Shadda with Superscript Alef Isolated Form
	{{0xFE7D, 0xFE77}, 0xFCF2}, // Shadda with Fatha Medial Form
	{{0xFE7D, 0xFE79}, 0xFCF3}, // Shadda with Damma Medial Form
	{{0xFE7D, 0xFE7B}, 0xFCF4}, // Shadda with Kasra Medial Form
};


/*
 * The Arabic diacritics causes the game to add a space,
 * for example "مَرحَب" which means hallo (written with 2 Fatḥah on it)
 * will look like "مَـ ـرحَـ ـب", please notice that we only need
 * Fatḥah (Fatha),
 * Kasrah (Kasra),
 * Ḍammah (Damma),
 * Sukun (Sukun),
 * Tanwin (the three of them) (Fathatan, Dammatan, Kasratan)
 * Shaddah (Shadda).
 */
const std::set<UChar> kArabicDiacritics = {
	{0x0670}, // Superscript Alef

	{0x0618}, // Small Fatha
	{0x064E}, // Fatha
	{0x065E}, // Fatha with Two Dots
	{0x08E4}, // Curly Fatha
	{0x08F4}, // Fatha with Ring
	{0x08F5}, // Fatha with Dot Above
	{0xFE76}, // Fatha Isolated Form
	{0xFE77}, // Fatha Medial Form

	{0x061A}, // Small Kasra
	{0x0650}, // Kasra
	{0x08E6}, // Curly Kasra
	{0x08F6}, // Kasra with Dot Below
	{0xFE7A}, // Kasra Isolated Form
	{0xFE7B}, // Kasra Medial Form

	{0x0619}, // Small Damma
	{0x064F}, // Damma
	{0x0657}, // Inverted Damma
	{0x065D}, // Reversed Damma
	{0x08E5}, // Curly Damma
	{0x08FE}, // Damma with Dot
	{0xFE78}, // Damma Isolated Form
	{0xFE79}, // Damma Medial Form

	{0x0652}, // Sukun
	{0xFE7E}, // Sukun Isolated Form
	{0xFE7F}, // Sukun Medial Form

	{0x08F0}, // Open Fathatan
	{0x064B}, // Fathatan
	{0x08E7}, // Curly Fathatan
	{0xFE70}, // Fathatan Isolated Form
	{0xFE71}, // Tatweel with Fathatan Above

	{0x08F1}, // Open Dammatan
	{0x064C}, // Dammatan
	{0x08E8}, // Curly Dammatan
	{0xFE72}, // Dammatan Isolated Form

	{0x08F2}, // Open Kasratan
	{0x064D}, // Kasratan
	{0x08E9}, // Curly Kasratan
	{0xFE74}, // Kasratan Isolated Form

	{0x0651}, // Shadda
	{0xFE7C}, // Shadda Isolated Form
	{0xFE7D}, // Shadda Medial Form

	{0xFC5E}, // Ligature Shadda with Dammatan Isolated Form
	{0xFC5F}, // Ligature Shadda with Kasratan Isolated Form
	{0xFC60}, // Ligature Shadda with Fatha Isolated Form
	{0xFC61}, // Ligature Shadda with Damma Isolated Form
	{0xFC62}, // Ligature Shadda with Kasra Isolated Form
	{0xFC63}, // Ligature Shadda with Superscript Alef Isolated Form
	{0xFCF2}, // Ligature Shadda with Fatha Medial Form
	{0xFCF3}, // Ligature Shadda with Damma Medial Form
	{0xFCF4}, // Ligature Shadda with Kasra Medial Form
};


// Map no longer used Arabic presentation forms to standard forms
const std::map<UChar, UChar> kArabicLegacyDiacritics = {
	// Isolated
	{0xFE76, 0x064E}, // Fatha
	{0xFE7A, 0x0650}, // Kasra
	{0xFE78, 0x064F}, // Damma
	{0xFE70, 0x064B}, // Fathatan
	{0xFE72, 0x064C}, // Dammatan
	{0xFE74, 0x064D}, // Kasratan
	{0xFE7C, 0x0651}, // Shadda

	// Medial
	{0xFE77, 0x064E}, // Fatha
	{0xFE7B, 0x0650}, // Kasra
	{0xFE79, 0x064F}, // Damma
	{0xFE7F, 0x0652}, // Sukun
	{0xFE7D, 0x0651}, // Shadda

	{0xFCF2, 0xFE76}, // Shadda with Fatha
	{0xFCF3, 0xFC61}, // Shadda with Damma
	{0xFCF4, 0xFC62}, // Shadda with Kasra
};


// Helper to printf ICU strings for testing
const char* icustring2char(icu::UnicodeString convertme) {
	std::string result;
	convertme.toUTF8String(result);
	return result.c_str();
}
const char* icuchar2char(UChar convertme) {
	icu::UnicodeString temp(convertme);
	return icustring2char(temp);
}

const std::set<std::string> kRTLScripts = {
	{"arabic", "devanagari", "hebrew", "mandaic", "nko", "samaritan", "syriac", "thaana"},
};

// http://unicode.org/faq/blocks_ranges.html
// http://unicode-table.com/en/blocks/
// TODO(GunChleoc): We might need some more here - let's see how this goes.
const std::map<std::string, std::set<UBlockCode>> kRTLCodeBlocks = {
	{"arabic", {
		 UBlockCode::UBLOCK_ARABIC,
		 UBlockCode::UBLOCK_ARABIC_SUPPLEMENT,
		 UBlockCode::UBLOCK_ARABIC_EXTENDED_A,
		 UBlockCode::UBLOCK_ARABIC_PRESENTATION_FORMS_A,
		 UBlockCode::UBLOCK_ARABIC_PRESENTATION_FORMS_B,
		 UBlockCode::UBLOCK_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS,
	 }},
	{"devanagari", {
		 UBlockCode::UBLOCK_DEVANAGARI,
		 UBlockCode::UBLOCK_DEVANAGARI_EXTENDED,
	 }},
	{"hebrew", {
		 UBlockCode::UBLOCK_HEBREW,
	 }},
	{"mandaic", {
		 UBlockCode::UBLOCK_MANDAIC,
	 }},
	{"nko", {
		 UBlockCode::UBLOCK_NKO,
	 }},
	{"samaritan", {
		 UBlockCode::UBLOCK_SAMARITAN,
	 }},
	{"syriac", {
		 UBlockCode::UBLOCK_SYRIAC,
	 }},
	{"thaana", {
		 UBlockCode::UBLOCK_THAANA,
	 }},
};

bool is_rtl_character(UChar32 c) {
	UBlockCode code = ublock_getCode(c);
	for (std::string script : kRTLScripts) {
		assert(kRTLCodeBlocks.count(script) == 1);
		if ((kRTLCodeBlocks.at(script).count(code) == 1)) {
			return true;
		}
	}
	return false;
}


} // namespace

namespace i18n {


// True if a string does not contain Latin characters
bool has_rtl_character(const char* input) {
	bool result = false;
	const icu::UnicodeString parseme(input);
	for (int32_t i = 0; i < parseme.length(); ++i) {
		if (is_rtl_character(parseme.char32At(i))) {
			result = true;
			break;
		}
	}
	return result;
}

// True if the strings do not contain Latin characters
bool has_rtl_character(std::vector<std::string> input) {
	bool result = false;
	for (const std::string& string: input) {
		if (has_rtl_character(string.c_str())) {
			result = true;
			break;
		}
	}
	return result;
}

// BiDi support for RTL languages
// Contracts glyphs into their ligatures
// NOCOM(GunChleoc): nūn (end) = FEE6, general = 0646, isolated = FEE5 is missing! cf. description for "The pass through the Mountains"
std::string make_ligatures(const char* input) {
	const icu::UnicodeString parseme(input);
	icu::UnicodeString queue;
	UChar not_a_character = 0xFFFF;
	UChar next = not_a_character;
	UChar previous = not_a_character;
	for (int i = 0; i < parseme.length(); ++i) {
		UChar c = parseme.charAt(i);

		// Substitution for Arabic characters
		previous = (i > 0) ? parseme.charAt(i - 1) : not_a_character;
		next = (i < (parseme.length() - 1)) ? parseme.charAt(i + 1) : not_a_character;

		if (kArabicDiacritics.count(c) == 1) {
			try {
				// Map legacy characters
				if (kArabicLegacyDiacritics.count(c) == 1) {
					c = kArabicLegacyDiacritics.at({c});
				}
				if (kArabicLegacyDiacritics.count(previous) == 1) {
					previous = kArabicLegacyDiacritics.at({previous});
				}
				if (kArabicLegacyDiacritics.count(next) == 1) {
					next = kArabicLegacyDiacritics.at({next});
				}

				// Special ligature forms combine 2 letters.
				if (kArabicLigatures.count({previous , c}) == 1) {
					c = kArabicLigatures.at({previous , c});
					// Now skip 1 letter, since we have just combined 2 letters
					++i;
					previous = (i > 0) ? parseme.charAt(i - 1) : not_a_character;
				}
			} catch (std::out_of_range e) {
				log("Error trying to fetch Arabic diacritic form: %s\n", e.what());
				assert(false);
			}
		} else if (kArabicFinalChars.count(c) == 1) { // All Arabic characters have a final form
			try {
				// Skip diacritics for position analysis
				for (int k = i - 2; k >= 0 && kArabicDiacritics.count(previous); --k) {
					previous = parseme.charAt(k);
				}
				for (int k = i + 2; k < (parseme.length() - 1) && kArabicDiacritics.count(next); ++k) {
					next = parseme.charAt(k);
				}

				// Special ligature forms combine 2 letters.
				if (kArabicLigatures.count({previous , c}) == 1) {
					c = kArabicLigatures.at({previous , c});
					// Now skip 1 letter, since we have just combined 2 letters
					++i;
					previous = (i > 0) ? parseme.charAt(i - 1) : not_a_character;
					// Skip diacritics for position analysis
					for (int k = i - 2; k >= 0 && kArabicDiacritics.count(previous); --k) {
						previous = parseme.charAt(k);
					}
				}

				// Arabic word characters have 4 forms to connect to each other:
				// Isolated, Initial, Medial, and Final.
				// - Isolated forms are the "canonical" forms and have no connection.
				// - Initial forms have a connection to the following letter.
				// - Medial forms have connections to both sides.
				// - Final forms have a connection to the preceding letter.
				// - All characters have an Isolated and a Final form.
				// - All characters that have a Medial form also have a Initial form and vice versa.
				// If a letter is in the middle of a word and the preceding letter has no connection pointing
				// towards it, pick the Initial or Isolated form (depending on if it is followed by another
				// letter) etc.
				if (kArabicFinalChars.count(previous) == 0) {  // Start of word
					if (kArabicInitialChars.count(c) == 1) {  // Link to next if character available
						c = kArabicInitialChars.at(c);
					}
				} else if (kArabicFinalChars.count(next) == 0) {  // End of word
					if (kArabicMedialChars.count(previous) == 1) {  // Link to previous if possible
						c = kArabicFinalChars.at(c);
					}
				} else { // Middle of word
					if (kArabicMedialChars.count(previous) == 1) {  // Link to previous if possible
						if (kArabicMedialChars.count(c) == 1) {
							c = kArabicMedialChars.at(c);
						} else {
							c = kArabicFinalChars.at(c);
						}
					} else {  // Link to next if character available
						if (kArabicInitialChars.count(c) == 1) {
							c = kArabicInitialChars.at(c);
						}
					}
				}
			} catch (std::out_of_range e) {
				log("Error trying to fetch Arabic character form: %s\n", e.what());
				assert(false);
			}
		}

		// TODO(GunChleoc): We sometimes get "Not a Character" - find out why.
		if (c != 0xFFFF) {
			// Add the current RTL character
			queue += c;
		}
	}

	std::string result;
	queue.toUTF8String(result);
	//log("NOCOM Ligatures result: %s\n", result.c_str());

	return result;
}


// BiDi support for RTL languages
// This turns the logical order of the glyphs into the display order.
std::string string2bidi(const char* input) {
	const icu::UnicodeString parseme(input);
	icu::UnicodeString stack;
	icu::UnicodeString temp_stack;
	UChar not_a_character = 0xFFFF;
	UChar previous = not_a_character;
	for (int i = parseme.length() - 1; i >= 0; --i) {
		UChar c = parseme.charAt(i);
		if (i > 0) {
			previous = parseme.charAt(i - 1);
		}
		// LTR characters need to be turned around in a temp stack,
		// because we will reverse everything on the bottom for RTL
		temp_stack = "";
		while (i >= 0 &&
				(is_numeric_char(c) ||
				 is_latin_char(c) ||
				 is_symmetric_char(c) ||
				 ((is_latin_char(previous) || is_punctuation_char(previous) ) && is_punctuation_char(c)))) {
			if (is_symmetric_char(c)) {
				c = mirror_symmetric_char(c);
			}
			temp_stack += c;
			c = parseme.charAt(--i);
			if (i > 0) {
				previous = parseme.charAt(i - 1);
			}
		}
		for (int32_t j = temp_stack.length() - 1; j >= 0; --j) {
			stack += temp_stack.charAt(j);
		}

		// TODO(GunChleoc): We sometimes get "Not a Character" - find out why.
		if (c != 0xFFFF) {
			// Add the current RTL character
			stack += c;
		}
	}

	std::string result;
	stack.toUTF8String(result);
	//log("NOCOM BiDI result: %s\n", result.c_str());

	return result;
}

} // namespace UI
