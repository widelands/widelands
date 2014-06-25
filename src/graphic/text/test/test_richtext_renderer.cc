/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include <memory>
#include <set>
#include <string>

#define BOOST_TEST_MODULE RichTextRendering

#include <SDL.h>
#include <SDL_TTF.h>
#undef main

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include "base/log.h"
#include "graphic/surface.h"
#include "graphic/text/test/paths.h"
#include "graphic/text/test/render.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"

struct RichTextTestFixture {
	RichTextTestFixture() : renderer(setup_standalone_renderer()) {}

	std::unique_ptr<StandaloneRenderer> renderer;
};


BOOST_FIXTURE_TEST_SUITE(richtext_test_suite, RichTextTestFixture)



namespace {

void EnsureSDLIsInitialized() {
	static bool done = false;
	if (!done) {
		SDL_Init(SDL_INIT_VIDEO);
		TTF_Init();
		done = true;
	}
}

int read_width(const std::string& basedir) {
	const std::string filename = basedir + "width";
	if (g_fs->FileExists(filename)) {
		FileRead fr;
		fr.Open(*g_fs, filename);
		return boost::lexical_cast<int>(fr.ReadLine());
	}
	return 0;
}

std::string read_file(const std::string& filename) {
	FileRead fr;
	fr.Open(*g_fs, filename);
	return std::string(fr.Data(fr.GetSize()), fr.GetSize());
}

bool CompareSurfaces(Surface* correct, Surface* generated) {
	if (correct->height() != generated->height()) {
		return false;
	}
	if (correct->width() != generated->width()) {
		return false;
	}

	correct->lock(Surface::Lock_Normal);
	generated->lock(Surface::Lock_Normal);

	bool are_equal = true;
	for (int y = 0; y < correct->height(); ++y) {
		for (int x = 0; x < correct->width(); ++x) {
			if (correct->get_pixel(x, y) != generated->get_pixel(x, y)) {
				are_equal = false;
			}
		}
	}

	generated->unlock(Surface::Unlock_NoChange);
	correct->unlock(Surface::Unlock_NoChange);
	return are_equal;
}

bool compare_for_test(StandaloneRenderer* renderer) {
	EnsureSDLIsInitialized();

	const std::string test_name = boost::unit_test::framework::current_test_case().p_name.value;
	const std::string basedir = std::string("data/") + test_name + "/";

	const int width = read_width(basedir);
	log("#sirver width: %u\n", width);

	std::unique_ptr<Surface> correct(renderer->image_loader()->load(basedir + "correct.png"));
	if (!correct) {
		log("Could not load %s/correct.png.", basedir.c_str());
		return false;
	}

	const std::set<std::string> inputs =
	   filter(g_fs->ListDirectory(basedir),
	          [](const std::string& fn) {return boost::contains(fn, "input");});
	for (const std::string& input : inputs) {
		const std::string input_text = read_file(input);
		std::unique_ptr<Surface> output(renderer->render(input_text, width));
		if (!CompareSurfaces(correct.get(), output.get())) {
			log("%s does not compare equal to correct.png.", input.c_str());
		}
	}
	return true;
}

}  // namespace

BOOST_AUTO_TEST_CASE(text_simple) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(br) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(bullet_point) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(escaped_text_simple) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_bold) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_color_blue) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_color_green) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_color_red) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_face) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_italic) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_ref) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_shadow) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_size_bigger) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_size_smaller) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_underline) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(font_various_attr) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(hline) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(hspace_dynamic1) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(hspace_dynamic2) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(hspace_dynamic_img) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(hspace_dynamic_text) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(hspace_fixed) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(img_simple) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(p_align) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(p_indent) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(p_spacing) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(p_valign_center) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(p_valign_default) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(p_valign_top) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(rt_bgcolor) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(rt_padding) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(rt_padding_allsites) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_autowidth_floatleftimg) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_background_img) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_fixedwidth_floatbothsides) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_fixedwidth_floatleft) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_fixedwidth_floatleftimg) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_fixedwidth_floatright) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_margin_bgclr) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_margin_bgimg) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_nonfloating_valign) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(sub_padding) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(table_like) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(text_linebreak) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(text_linebreak01) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(text_linebreak02) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(text_simple_autowidth) {BOOST_CHECK(compare_for_test(renderer.get()));}
BOOST_AUTO_TEST_CASE(vspace) {BOOST_CHECK(compare_for_test(renderer.get()));}

BOOST_AUTO_TEST_SUITE_END();
