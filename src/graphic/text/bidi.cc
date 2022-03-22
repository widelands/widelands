/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "graphic/text/bidi.h"

#include <unicode/utypes.h>

#include "base/log.h"
#include "base/wexception.h"
#include "graphic/text/font_set.h"

namespace {
// TODO(GunChleoc): Have a look at the ICU API to see which helper functions can be gained from
// there.
// TODO(GunChleoc): Arabic: Turn this into a proper class

// https://www.w3.org/TR/jlreq/#characters_not_starting_a_line
const std::set<UChar> kCannottStartLineJapanese = {
   0x2019,  // RIGHT SINGLE QUOTATION MARK
   0x201D,  // RIGHT DOUBLE QUOTATION MARK
   0x0029,  // RIGHT PARENTHESIS
   0x3015,  // RIGHT TORTOISE SHELL BRACKET
   0x005D,  // RIGHT SQUARE BRACKET
   0x007D,  // RIGHT CURLY BRACKET
   0x3009,  // RIGHT ANGLE BRACKET
   0x300B,  // RIGHT DOUBLE ANGLE BRACKET
   0x300D,  // RIGHT CORNER BRACKET
   0x300F,  // RIGHT WHITE CORNER BRACKET
   0x3011,  // RIGHT BLACK LENTICULAR BRACKET
   0x2986,  // RIGHT WHITE PARENTHESIS
   0x3019,  // RIGHT WHITE TORTOISE SHELL BRACKET
   0x3017,  // RIGHT WHITE LENTICULAR BRACKET
   0xFF09,  // Fullwidth Right Parenthesis
   0x00BB,  // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
   0x301F,  // LOW DOUBLE PRIME QUOTATION MARK
   0x2010,  // HYPHEN
   0x301C,  // WAVE DASH
   0x30A0,  // KATAKANA-HIRAGANA DOUBLE HYPHEN
   0x2013,  // EN DASH
   0x0021,  // EXCLAMATION MARK
   0x003F,  // QUESTION MARK
   0x203C,  // DOUBLE EXCLAMATION MARK
   0x2047,  // DOUBLE QUESTION MARK
   0x2048,  // QUESTION EXCLAMATION MARK
   0x2049,  // EXCLAMATION QUESTION MARK
   0x30FB,  // KATAKANA MIDDLE DOT
   0x003A,  // COLON
   0x003B,  // SEMICOLON
   0x3002,  // IDEOGRAPHIC FULL STOP
   0x002E,  // FULL STOP
   0x3001,  // IDEOGRAPHIC COMMA
   0x002C,  // COMMA
   0x30FD,  // KATAKANA ITERATION MARK
   0x30FE,  // KATAKANA VOICED ITERATION MARK
   0x309D,  // HIRAGANA ITERATION MARK
   0x309E,  // HIRAGANA VOICED ITERATION MARK
   0x3005,  // IDEOGRAPHIC ITERATION MARK
   0x303B,  // VERTICAL IDEOGRAPHIC ITERATION MARK
   0x30FC,  // KATAKANA-HIRAGANA PROLONGED SOUND MARK
   0x3041,  // HIRAGANA LETTER SMALL A
   0x3043,  // HIRAGANA LETTER SMALL I
   0x3045,  // HIRAGANA LETTER SMALL U
   0x3047,  // HIRAGANA LETTER SMALL E
   0x3049,  // HIRAGANA LETTER SMALL O
   0x30A1,  // KATAKANA LETTER SMALL A
   0x30A3,  // KATAKANA LETTER SMALL I
   0x30A5,  // KATAKANA LETTER SMALL U
   0x30A7,  // KATAKANA LETTER SMALL E
   0x30A9,  // KATAKANA LETTER SMALL O
   0x3063,  // HIRAGANA LETTER SMALL TU
   0x3083,  // HIRAGANA LETTER SMALL YA
   0x3085,  // HIRAGANA LETTER SMALL YU
   0x3087,  // HIRAGANA LETTER SMALL YO
   0x308E,  // HIRAGANA LETTER SMALL WA
   0x3095,  // HIRAGANA LETTER SMALL KA
   0x3096,  // HIRAGANA LETTER SMALL KE
   0x30C3,  // KATAKANA LETTER SMALL TU
   0x30E3,  // KATAKANA LETTER SMALL YA
   0x30E5,  // KATAKANA LETTER SMALL YU
   0x30E7,  // KATAKANA LETTER SMALL YO
   0x30EE,  // KATAKANA LETTER SMALL WA
   0x30F5,  // KATAKANA LETTER SMALL KA
   0x30F6,  // KATAKANA LETTER SMALL KE
   0x31F0,  // KATAKANA LETTER SMALL KU
   0x31F1,  // KATAKANA LETTER SMALL SI
   0x31F2,  // KATAKANA LETTER SMALL SU
   0x31F3,  // KATAKANA LETTER SMALL TO
   0x31F4,  // KATAKANA LETTER SMALL NU
   0x31F5,  // KATAKANA LETTER SMALL HA
   0x31F6,  // KATAKANA LETTER SMALL HI
   0x31F7,  // KATAKANA LETTER SMALL HU
   0x31F8,  // KATAKANA LETTER SMALL HE
   0x31F9,  // KATAKANA LETTER SMALL HO
   0x31FA,  // KATAKANA LETTER SMALL MU
   0x31FB,  // KATAKANA LETTER SMALL RA
   0x31FC,  // KATAKANA LETTER SMALL RI
   0x31FD,  // KATAKANA LETTER SMALL RU
   0x31FE,  // KATAKANA LETTER SMALL RE
   0x31FF,  // KATAKANA LETTER SMALL RO
};

// https://www.w3.org/TR/jlreq/#characters_not_ending_a_line
const std::set<UChar> kCannotEndLineJapanese = {
   0x2018,  // LEFT SINGLE QUOTATION MARK
   0x201C,  // LEFT DOUBLE QUOTATION MARK
   0x0028,  // LEFT PARENTHESIS
   0x3014,  // LEFT TORTOISE SHELL BRACKET
   0x005B,  // LEFT SQUARE BRACKET
   0x007B,  // LEFT CURLY BRACKET
   0x3008,  // LEFT ANGLE BRACKET
   0x300A,  // LEFT DOUBLE ANGLE BRACKET
   0x300C,  // LEFT CORNER BRACKET
   0x300E,  // LEFT WHITE CORNER BRACKET
   0x3010,  // LEFT BLACK LENTICULAR BRACKET
   0x2985,  // LEFT WHITE PARENTHESIS
   0x3018,  // LEFT WHITE TORTOISE SHELL BRACKET
   0x3016,  // LEFT WHITE LENTICULAR BRACKET
   0xFF08,  // Fullwidth Left Parenthesis
   0x00AB,  // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
   0x301D,  // REVERSED DOUBLE PRIME QUOTATION MARK
};

// Need to mirror () etc. for LTR languages, so we're sticking them in a map.
const std::map<UChar, UChar> kSymmetricChars = {
   {0x0028, 0x0029},  // ()
   {0x0029, 0x0028},  // )(
   {0x003C, 0x003E},  // <>
   {0x003E, 0x003C},  // ><
   {0x005B, 0x005D},  // []
   {0x005D, 0x005B},  // ][
   {0x007B, 0x007D},  // {}
   {0x007D, 0x007B},  // }{
   {0x201C, 0x201D},  // “”
   {0x201D, 0x201C},  // ”“
   {0x2018, 0x2019},  // ‘’
   {0x2019, 0x2018},  // ’‘
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
	return c == 0x0020 ||                 // blank space
	       c == 0x0021 ||                 // !
	       c == 0x002C ||                 // ,
	       c == 0x002D ||                 // -
	       c == 0x002E ||                 // .
	       c == 0x002F ||                 // /
	       c == 0x003A ||                 // :
	       c == 0x003B ||                 // ;
	       c == 0x003F ||                 // ?
	       c == 0x005C ||                 // backslash
	       (c >= 0x2000 && c <= 0x206F);  // en-dash, em-dash etc.
}

// TODO(GunChleoc): Presentation forms A on demand
// https://unicode-table.com/en/blocks/arabic-presentation-forms-a/
const std::map<UChar, UChar> kArabicFinalChars = {
   {0x0622, 0xFE82},  // ʾalif maddah
   {0x0623, 0xFE84},  // ʾalif with hamza above
   {0x0624, 0xFE86},  // wāw with hamza above
   {0x0625, 0xFE88},  // ʾalif with hamza below
   {0x0626, 0xFE8A},  // yāʾ with hamza above
   {0x0627, 0xFE8E},  // ʾalif
   {0x0628, 0xFE90},  // bāʾ
   {0x0629, 0xFE94},  // tāʾ marbūṭah
   {0x062A, 0xFE96},  // tāʾ
   {0x062B, 0xFE9A},  // ṯāʾ
   {0x062C, 0xFE9E},  // ǧīm
   {0x062D, 0xFEA2},  // ḥāʾ
   {0x062E, 0xFEA6},  // ḫāʾ
   {0x062F, 0xFEAA},  // dāl
   {0x0630, 0xFEAC},  // ḏāl
   {0x0631, 0xFEAE},  // rāʾ
   {0x0632, 0xFEB0},  // zayn/zāy
   {0x0633, 0xFEB2},  // sīn
   {0x0634, 0xFEB6},  // šīn
   {0x0635, 0xFEBA},  // ṣād
   {0x0636, 0xFEBE},  // ḍād
   {0x0637, 0xFEC2},  // ṭāʾ
   {0x0638, 0xFEC6},  // ẓāʾ
   {0x0639, 0xFECA},  // ʿayn
   {0x063A, 0xFECE},  // ġayn
   /*
   {0x063B, 0x}, // Keheh with Two Dots Above
   {0x063C, 0x}, // Keheh with Three Dots Below
   {0x063D, 0x}, // Farsi Yeh with Inverted V
   {0x063E, 0x}, // Farsi Yeh with Two Dots Above
   {0x063F, 0x}, // Farsi Yeh with Three Dots Above
      */
   {0x0641, 0xFED2},  // fāʾ
   {0x0642, 0xFED6},  // qāf
   {0x0643, 0xFEDA},  // kāf
   {0x0644, 0xFEDE},  // lām
   {0x0645, 0xFEE2},  // mīm
   {0x0646, 0xFEE6},  // nūn
   {0x0647, 0xFEEA},  // hāʾ
   {0x0648, 0xFEEE},  // wāw
   {0x0649, 0xFEF0},  // ʾalif maqṣūrah
   {0x064A, 0xFEF2},  // yāʾ
   {0xFD3D, 0xFD3C},  // Ligature Alef with Fathatan
   {0xFEF5, 0xFEF6},  // lām ʾalif maddah
   {0xFEF7, 0xFEF8},  // lām ʾalif hamza above
   {0xFEF9, 0xFEFA},  // lām ʾalif hamza below
   {0xFEFB, 0xFEFC},  // lām ʾalif
};

const std::map<UChar, UChar> kArabicInitialChars = {
   {0x0626, 0xFE8B},  // yāʾ with hamza above
   {0x0628, 0xFE91},  // bāʾ
   {0x062A, 0xFE97},  // tāʾ
   {0x062B, 0xFE9B},  // ṯāʾ
   {0x062C, 0xFE9F},  // ǧīm
   {0x062D, 0xFEA3},  // ḥāʾ
   {0x062E, 0xFEA7},  // ḫāʾ
   {0x0633, 0xFEB3},  // sīn
   {0x0634, 0xFEB7},  // šīn
   {0x0635, 0xFEBB},  // ṣād
   {0x0636, 0xFEBF},  // ḍād
   {0x0637, 0xFEC3},  // ṭāʾ
   {0x0638, 0xFEC7},  // ẓāʾ
   {0x0639, 0xFECB},  // ʿayn
   {0x063A, 0xFECF},  // ġayn
   {0x0641, 0xFED3},  // fāʾ
   {0x0642, 0xFED7},  // qāf
   {0x0643, 0xFEDB},  // kāf
   {0x0644, 0xFEDF},  // lām
   {0x0645, 0xFEE3},  // mīm
   {0x0646, 0xFEE7},  // nūn
   {0x0647, 0xFEEB},  // hāʾ
   {0x064A, 0xFEF3},  // yāʾ
};

const std::map<UChar, UChar> kArabicMedialChars = {
   {0x0626, 0xFE8C},  // yāʾ with hamza above
   {0x0628, 0xFE92},  // bāʾ
   {0x062A, 0xFE98},  // tāʾ
   {0x062B, 0xFE9C},  // ṯāʾ
   {0x062C, 0xFEA0},  // ǧīm
   {0x062D, 0xFEA4},  // ḥāʾ
   {0x062E, 0xFEA8},  // ḫāʾ
   {0x0633, 0xFEB4},  // sīn
   {0x0634, 0xFEB8},  // šīn
   {0x0635, 0xFEBC},  // ṣād
   {0x0636, 0xFEC0},  // ḍād
   {0x0637, 0xFEC4},  // ṭāʾ
   {0x0638, 0xFEC8},  // ẓāʾ
   {0x0639, 0xFECC},  // ʿayn
   {0x063A, 0xFED0},  // ġayn
   {0x0641, 0xFED4},  // fāʾ
   {0x0642, 0xFED8},  // qāf
   {0x0643, 0xFEDC},  // kāf
   {0x0644, 0xFEE0},  // lām
   {0x0645, 0xFEE4},  // mīm
   {0x0646, 0xFEE8},  // nūn
   {0x0647, 0xFEEC},  // hāʾ
   {0x064A, 0xFEF4},  // yāʾ
};

// Special ligature forms combine 2 letters. lām-alif ligature is mandatory.
// Diacritics also form ligatures.
const std::map<std::pair<UChar, UChar>, UChar> kArabicLigatures = {
   {{0x0644, 0x0622}, 0xFEF5},  // lām ʾalif maddah
   {{0x0644, 0x0623}, 0xFEF7},  // lām ʾalif hamza above
   {{0x0644, 0x0625}, 0xFEF9},  // lām ʾalif hamza below
   {{0x0644, 0x0627}, 0xFEFB},  // lām ʾalif
   {{0x0627, 0x064B}, 0xFD3D},  // Ligature Alef with Fathatan Isolated Form

   // Basing these off the isolated forms; I don't know what will happen with canonical forms here
   {{0xFE7C, 0xFE72}, 0xFC5E},  // Shadda with Dammatan Isolated Form
   {{0xFE7C, 0xFE74}, 0xFC5F},  // Shadda with Kasratan Isolated Form
   {{0xFE7C, 0x064D}, 0xFE76},  // Shadda with Fatha Isolated Form
   {{0xFE7C, 0xFE78}, 0xFC61},  // Shadda with Damma Isolated Form
   {{0xFE7C, 0xFE7A}, 0xFC62},  // Shadda with Kasra Isolated Form
   {{0xFE7C, 0x0670}, 0xFC63},  // Shadda with Superscript Alef Isolated Form
   {{0xFE7D, 0xFE77}, 0xFCF2},  // Shadda with Fatha Medial Form
   {{0xFE7D, 0xFE79}, 0xFCF3},  // Shadda with Damma Medial Form
   {{0xFE7D, 0xFE7B}, 0xFCF4},  // Shadda with Kasra Medial Form
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
   0x0670,  // Superscript Alef

   0x0618,  // Small Fatha
   0x064E,  // Fatha
   0x065E,  // Fatha with Two Dots
   0x08E4,  // Curly Fatha
   0x08F4,  // Fatha with Ring
   0x08F5,  // Fatha with Dot Above
   0xFE76,  // Fatha Isolated Form
   0xFE77,  // Fatha Medial Form

   0x061A,  // Small Kasra
   0x0650,  // Kasra
   0x08E6,  // Curly Kasra
   0x08F6,  // Kasra with Dot Below
   0xFE7A,  // Kasra Isolated Form
   0xFE7B,  // Kasra Medial Form

   0x0619,  // Small Damma
   0x064F,  // Damma
   0x0657,  // Inverted Damma
   0x065D,  // Reversed Damma
   0x08E5,  // Curly Damma
   0x08FE,  // Damma with Dot
   0xFE78,  // Damma Isolated Form
   0xFE79,  // Damma Medial Form

   0x0652,  // Sukun
   0xFE7E,  // Sukun Isolated Form
   0xFE7F,  // Sukun Medial Form

   0x08F0,  // Open Fathatan
   0x064B,  // Fathatan
   0x08E7,  // Curly Fathatan
   0xFE70,  // Fathatan Isolated Form
   0xFE71,  // Tatweel with Fathatan Above

   0x08F1,  // Open Dammatan
   0x064C,  // Dammatan
   0x08E8,  // Curly Dammatan
   0xFE72,  // Dammatan Isolated Form

   0x08F2,  // Open Kasratan
   0x064D,  // Kasratan
   0x08E9,  // Curly Kasratan
   0xFE74,  // Kasratan Isolated Form

   0x0651,  // Shadda
   0xFE7C,  // Shadda Isolated Form
   0xFE7D,  // Shadda Medial Form

   0xFC5E,  // Ligature Shadda with Dammatan Isolated Form
   0xFC5F,  // Ligature Shadda with Kasratan Isolated Form
   0xFC60,  // Ligature Shadda with Fatha Isolated Form
   0xFC61,  // Ligature Shadda with Damma Isolated Form
   0xFC62,  // Ligature Shadda with Kasra Isolated Form
   0xFC63,  // Ligature Shadda with Superscript Alef Isolated Form
   0xFCF2,  // Ligature Shadda with Fatha Medial Form
   0xFCF3,  // Ligature Shadda with Damma Medial Form
   0xFCF4,  // Ligature Shadda with Kasra Medial Form
};

// Map no longer used Arabic presentation forms to standard forms
const std::map<UChar, UChar> kArabicLegacyDiacritics = {
   // Isolated
   {0xFE76, 0x064E},  // Fatha
   {0xFE7A, 0x0650},  // Kasra
   {0xFE78, 0x064F},  // Damma
   {0xFE70, 0x064B},  // Fathatan
   {0xFE72, 0x064C},  // Dammatan
   {0xFE74, 0x064D},  // Kasratan
   {0xFE7C, 0x0651},  // Shadda

   // Medial
   {0xFE77, 0x064E},  // Fatha
   {0xFE7B, 0x0650},  // Kasra
   {0xFE79, 0x064F},  // Damma
   {0xFE7F, 0x0652},  // Sukun
   {0xFE7D, 0x0651},  // Shadda

   {0xFCF2, 0xFE76},  // Shadda with Fatha
   {0xFCF3, 0xFC61},  // Shadda with Damma
   {0xFCF4, 0xFC62},  // Shadda with Kasra
};

const std::set<UI::FontSets::Selector> kLTRScripts = {
   // We omit the default fontset, because we won't define code blocks for it - it's a catch-all.
   UI::FontSets::Selector::kCJK};

// https://unicode.org/faq/blocks_ranges.html
// https://unicode-table.com/en/blocks/
const std::map<UI::FontSets::Selector, std::set<UBlockCode>> kLTRCodeBlocks = {
   {UI::FontSets::Selector::kCJK,
    {
       UBlockCode::UBLOCK_CJK_COMPATIBILITY,
       UBlockCode::UBLOCK_CJK_COMPATIBILITY_FORMS,
       UBlockCode::UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS,
       UBlockCode::UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT,
       UBlockCode::UBLOCK_CJK_RADICALS_SUPPLEMENT,
       UBlockCode::UBLOCK_CJK_STROKES,
       UBlockCode::UBLOCK_CJK_SYMBOLS_AND_PUNCTUATION,
       UBlockCode::UBLOCK_CJK_UNIFIED_IDEOGRAPHS,
       UBlockCode::UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A,
       UBlockCode::UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B,
       UBlockCode::UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C,
       UBlockCode::UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D,
       UBlockCode::UBLOCK_HIRAGANA,
       UBlockCode::UBLOCK_KATAKANA,
       UBlockCode::UBLOCK_KATAKANA_PHONETIC_EXTENSIONS,
       UBlockCode::UBLOCK_ENCLOSED_CJK_LETTERS_AND_MONTHS,
       UBlockCode::UBLOCK_HANGUL_COMPATIBILITY_JAMO,
       UBlockCode::UBLOCK_HANGUL_JAMO,
       UBlockCode::UBLOCK_HANGUL_JAMO_EXTENDED_A,
       UBlockCode::UBLOCK_HANGUL_JAMO_EXTENDED_B,
       UBlockCode::UBLOCK_HANGUL_SYLLABLES,
    }}};

const std::set<UI::FontSets::Selector> kRTLScripts = {
   // Add "mandaic", "nko", "samaritan", "syriac", "thaana" if we get these languages.
   UI::FontSets::Selector::kArabic, UI::FontSets::Selector::kDevanagari,
   UI::FontSets::Selector::kHebrew};

// https://unicode.org/faq/blocks_ranges.html
// https://unicode-table.com/en/blocks/
// TODO(GunChleoc): We might need some more here - let's see how this goes.
const std::map<UI::FontSets::Selector, std::set<UBlockCode>> kRTLCodeBlocks = {
   {UI::FontSets::Selector::kArabic,
    {
       UBlockCode::UBLOCK_ARABIC,
       UBlockCode::UBLOCK_ARABIC_SUPPLEMENT,
       UBlockCode::UBLOCK_ARABIC_EXTENDED_A,
       UBlockCode::UBLOCK_ARABIC_PRESENTATION_FORMS_A,
       UBlockCode::UBLOCK_ARABIC_PRESENTATION_FORMS_B,
       UBlockCode::UBLOCK_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS,
    }},
   {UI::FontSets::Selector::kDevanagari,
    {
       UBlockCode::UBLOCK_DEVANAGARI,
       UBlockCode::UBLOCK_DEVANAGARI_EXTENDED,
       UBlockCode::UBLOCK_VEDIC_EXTENSIONS,
    }},
   {UI::FontSets::Selector::kHebrew,
    {
       UBlockCode::UBLOCK_HEBREW,
    }},
   /* Activate when we get any of these languages
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
       */
};

// True if the character is in one of the script's code blocks
bool is_script_character(UChar32 c, UI::FontSets::Selector script) {
	CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
	UBlockCode code = ublock_getCode(c);
	CLANG_DIAG_ON("-Wdisabled-macro-expansion")
	if (kRTLCodeBlocks.count(script) == 1 && kRTLCodeBlocks.at(script).count(code) == 1) {
		return true;
	}
	if (kLTRCodeBlocks.count(script) == 1 && kLTRCodeBlocks.at(script).count(code) == 1) {
		return true;
	}
	return false;
}

bool is_rtl_character(UChar32 c) {
	CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
	UBlockCode code = ublock_getCode(c);
	CLANG_DIAG_ON("-Wdisabled-macro-expansion")
	for (UI::FontSets::Selector script : kRTLScripts) {
		assert(kRTLCodeBlocks.count(script) == 1);
		if ((kRTLCodeBlocks.at(script).count(code) == 1)) {
			return true;
		}
	}
	return false;
}

// Helper function for make_ligatures.
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
UChar find_arabic_letter_form(UChar c, UChar previous, UChar next) {
	if (kArabicFinalChars.count(previous) == 0) {  // Start of word
		if (kArabicInitialChars.count(c) == 1) {    // Link to next if character available
			c = kArabicInitialChars.at(c);
		}
	} else if (kArabicFinalChars.count(next) == 0) {   // End of word
		if (kArabicMedialChars.count(previous) == 1) {  // Link to previous if possible
			c = kArabicFinalChars.at(c);
		}
	} else {                                           // Middle of word
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
	return c;
}

}  // namespace

namespace i18n {

// True if a string does not contain Latin characters.
// Checks for the first 'limit' characters maximum.
bool has_rtl_character(const char* input, int32_t limit) {
	const icu::UnicodeString parseme(input, "UTF-8");
	for (int32_t i = 0; i < parseme.length() && i < limit; ++i) {
		if (is_rtl_character(parseme.char32At(i))) {
			return true;
		}
	}
	return false;
}

// True if the strings do not contain Latin characters
bool has_rtl_character(std::vector<std::string> input) {
	for (const std::string& string : input) {
		if (has_rtl_character(string.c_str())) {
			return true;
		}
	}
	return false;
}

// Contracts glyphs into their ligatures
std::string make_ligatures(const char* input) {
	// We only have defined ligatures for Arabic at the moment.
	if (!has_script_character(input, UI::FontSets::Selector::kArabic)) {
		return input;
	}
	const icu::UnicodeString parseme(input, "UTF-8");
	icu::UnicodeString queue;
	const UChar not_a_character = 0xFFFF;
	UChar next;
	UChar previous;
	for (int i = parseme.length() - 1; i >= 0; --i) {
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

				// Special ligature forms combine 2 letters.
				if (kArabicLigatures.count({previous, c}) == 1) {
					c = kArabicLigatures.at({previous, c});
					// Now skip 1 letter, since we have just combined 2 letters
					--i;
				}
			} catch (const std::out_of_range& e) {
				log_err("Error trying to fetch Arabic diacritic form: %s\n", e.what());
				NEVER_HERE();
			}
		} else if (kArabicFinalChars.count(c) == 1) {  // All Arabic characters have a final form
			try {
				// Skip diacritics for position analysis
				for (int k = i - 2; k >= 0 && (kArabicDiacritics.count(previous) != 0u); --k) {
					previous = parseme.charAt(k);
				}
				for (int k = i + 2; k < (parseme.length() - 1) && (kArabicDiacritics.count(next) != 0u);
				     ++k) {
					next = parseme.charAt(k);
				}

				// Special ligature forms combine 2 letters.
				if (kArabicLigatures.count({previous, c}) == 1) {
					c = kArabicLigatures.at({previous, c});
					// Now skip 1 letter, since we have just combined 2 letters
					--i;
					previous = (i > 0) ? parseme.charAt(i - 1) : not_a_character;
					// Skip diacritics for position analysis
					for (int k = i - 2; k >= 0 && (kArabicDiacritics.count(previous) != 0u); --k) {
						previous = parseme.charAt(k);
					}
				}
				c = find_arabic_letter_form(c, previous, next);
			} catch (const std::out_of_range& e) {
				log_err("Error trying to fetch Arabic character form: %s\n", e.what());
				NEVER_HERE();
			}
		}

		// TODO(GunChleoc): We sometimes get "Not a Character" - find out why.
		if (c != 0xFFFF) {
			// Add the current RTL character
			queue.insert(0, c);
		}
	}

	std::string result;
	queue.toUTF8String(result);
	return result;
}

// BiDi support for RTL languages
// This turns the logical order of the glyphs into the display order.
std::string line2bidi(const char* input) {
	const icu::UnicodeString parseme(input, "UTF-8");
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
		while (i >= 0 && (is_numeric_char(c) || is_latin_char(c) || is_symmetric_char(c) ||
		                  ((is_latin_char(previous) || is_punctuation_char(previous)) &&
		                   is_punctuation_char(c)))) {
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
	return result;
}

// Helper to convert ICU strings to C++ strings
std::string icustring2string(const icu::UnicodeString& convertme) {
	std::string result;
	convertme.toUTF8String(result);
	return result;
}

// True if a string contains a character from the script's code blocks
bool has_script_character(const char* input, UI::FontSets::Selector script) {
	const icu::UnicodeString parseme(input, "UTF-8");
	for (int32_t i = 0; i < parseme.length(); ++i) {
		if (is_script_character(parseme.char32At(i), script)) {
			return true;
		}
	}
	return false;
}

UI::FontSet const* find_fontset(const char* word, const UI::FontSets& fontsets) {
	UI::FontSets::Selector selector;
	if (has_script_character(word, UI::FontSets::Selector::kArabic)) {
		selector = UI::FontSets::Selector::kArabic;
	} else if (has_script_character(word, UI::FontSets::Selector::kCJK)) {
		selector = UI::FontSets::Selector::kCJK;
	} else if (has_script_character(word, UI::FontSets::Selector::kDevanagari)) {
		selector = UI::FontSets::Selector::kDevanagari;
	} else if (has_script_character(word, UI::FontSets::Selector::kHebrew)) {
		selector = UI::FontSets::Selector::kHebrew;
	} else {
		selector = UI::FontSets::Selector::kDefault;
	}
	return fontsets.get_fontset(selector);
}

//  Split a string of CJK characters into units that can have line breaks between them.
std::vector<std::string> split_cjk_word(const char* input) {
	const icu::UnicodeString parseme(input, "UTF-8");
	std::vector<std::string> result;
	for (int i = 0; i < parseme.length(); ++i) {
		icu::UnicodeString temp;
		UChar c = parseme.charAt(i);
		temp += c;
		if (i < parseme.length() - 1) {
			UChar next = parseme.charAt(i + 1);
			if (cannot_end_line(c) || cannot_start_line(next)) {
				temp += next;
				++i;
			}
		}
		std::string temp2;
		result.push_back(temp.toUTF8String(temp2));
	}
	return result;
}

bool cannot_start_line(const UChar& c) {
	return is_diacritic(c) || is_punctuation_char(c) || kCannottStartLineJapanese.count(c) == 1;
}

bool cannot_end_line(const UChar& c) {
	return kCannotEndLineJapanese.count(c) == 1;
}

bool is_diacritic(const UChar& c) {
	return kArabicDiacritics.count(c) == 1;
}

}  // namespace i18n
