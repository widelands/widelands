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
#include <SDL_image.h>
#include <SDL_ttf.h>
#undef main

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include "base/log.h"
#include "graphic/image_io.h"
#include "graphic/render/sdl_surface.h"
#include "graphic/surface.h"
#include "graphic/text/rt_errors.h"
#include "graphic/text/test/paths.h"
#include "graphic/text/test/render.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "io/streamwrite.h"

struct RichTextTestFixture {
	StandaloneRenderer sar;
};

BOOST_FIXTURE_TEST_SUITE(richtext_test_suite, RichTextTestFixture)

namespace {

struct RefMapTestSample {
	int x, y;
	std::string expected_text;
};

int hypot_sqr(int x, int y) {
	return (x - y) * (x - y);
}

void ensure_sdl_is_initialized() {
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

bool read_ref_map(const std::string& basedir, std::vector<RefMapTestSample>* samples) {
	const std::string filename = basedir + "ref_map";
	if (!g_fs->FileExists(filename)) {
		return true;
	}
	FileRead fr;
	fr.Open(*g_fs, filename);
	while (!fr.EndOfFile()) {
		const std::string current_line = fr.ReadLine();
		const size_t beginning_of_string = current_line.find('"');
		const size_t end_of_string = current_line.rfind('"');

		std::vector<std::string> position_strings;
		const std::string numbers = current_line.substr(0, beginning_of_string - 1);
		boost::algorithm::split(position_strings, numbers, boost::is_any_of(" "));
		if (position_strings.size() != 2) {
			log("%s/ref_map contains invalid line '%s'.\n", basedir.c_str(), current_line.c_str());
			return false;
		}

		samples->push_back(RefMapTestSample());
		samples->back().expected_text =
		   current_line.substr(beginning_of_string + 1, end_of_string - beginning_of_string - 1);
		samples->back().x = boost::lexical_cast<int>(position_strings[0]);
		samples->back().y = boost::lexical_cast<int>(position_strings[1]);
	}
	return true;
}

std::string read_file(const std::string& filename) {
	FileRead fr;
	fr.Open(*g_fs, filename);
	return std::string(fr.Data(fr.GetSize()), fr.GetSize());
}

bool compare_surfaces(Surface* correct, Surface* generated) {
	if (correct->height() != generated->height()) {
		log(" correct->height() != generated->height(): (%d, %d)\n",
		    correct->height(),
		    generated->height());
		return false;
	}
	if (correct->width() != generated->width()) {
		log(" correct->width() != generated->width(): (%d, %d)\n",
		    correct->width(),
		    generated->width());
		return false;
	}

	// Saving a PNG slightly changes some pixel values. So we save and reload
	// the generated PNG in memory to get the same effects on the generated
	// image too.
	FileWrite fw;
	if (!save_surface_to_png(generated, &fw)) {
		std::cout << "Could not encode PNG." << std::endl;
	}

	const std::string encoded_data = fw.GetData();
	std::unique_ptr<SDLSurface> converted(new SDLSurface(IMG_Load_RW(
	   SDL_RWFromConstMem(encoded_data.data(), encoded_data.size()), false /* free source */)));

	correct->lock(Surface::Lock_Normal);
	converted->lock(Surface::Lock_Normal);

	int nwrong = 0;
	for (int y = 0; y < correct->height(); ++y) {
		for (int x = 0; x < correct->width(); ++x) {
			RGBAColor cclr, gclr;
			SDL_GetRGBA(
			   correct->get_pixel(x, y), &correct->format(), &cclr.r, &cclr.g, &cclr.b, &cclr.a);
			SDL_GetRGBA(
			   converted->get_pixel(x, y), &converted->format(), &gclr.r, &gclr.g, &gclr.b, &gclr.a);

			if (cclr == gclr) {
				continue;
			}

			// Somehow a black pixel can have different alpha values. Probably
			// because it does not matter when blending anyways.
			// if (cclr.r == gclr.r && cclr.r == 0 && cclr.g == gclr.g && cclr.g == 0 &&
				 // cclr.b == gclr.b && cclr.b == 0) {
				// // Only alpha differs. Ignore.
				// continue;
			// }

			// NOCOM(#sirver): bring back or remove.
			// // But that is still not enough, so we let a minimum distance to be allowed.
			// // This might need tweaking to work on all systems.
			// const int distance = hypot_sqr(cclr.r, gclr.r) + hypot_sqr(cclr.g, gclr.g) +
										// hypot_sqr(cclr.b, gclr.b) + hypot_sqr(cclr.a, gclr.a);
			// constexpr int kMaxAllowedSquaredPixelDistance = 6;
			// if (distance >= kMaxAllowedSquaredPixelDistance) {
			log("Mismatched pixel: (%d, %d)\n", x, y);
			log(" expected: (%d, %d, %d, %d)\n", cclr.r, cclr.g, cclr.b, cclr.a);
			log(" seen:     (%d, %d, %d, %d)\n", gclr.r, gclr.g, gclr.b, gclr.a);
			// log(" distances: %d, allowed: %d\n\n", distance, kMaxAllowedSquaredPixelDistance);

			++nwrong;
			// }
		}
	}

	converted->unlock(Surface::Unlock_Update);
	correct->unlock(Surface::Unlock_NoChange);

	if (nwrong > 0) {
		log(" wrong pixels: %.2f %%\n",
		    static_cast<float>(nwrong) / (correct->width() * correct->height()));
	}
	return nwrong == 0;
}

bool compare_for_test(RT::Renderer* renderer) {
	ensure_sdl_is_initialized();

	const std::string test_name = boost::unit_test::framework::current_test_case().p_name.value;
	const std::string basedir = std::string("data/") + test_name + "/";

	const int width = read_width(basedir);

	std::vector<RefMapTestSample> ref_map_samples;
	if (!read_ref_map(basedir, &ref_map_samples)) {
		log("Could not load %s/ref_map.\n", basedir.c_str());
		return false;
	}

	std::unique_ptr<Surface> correct;

	const std::set<std::string> inputs =
	   filter(g_fs->ListDirectory(basedir),
	          [](const std::string& fn) {return boost::contains(fn, "input");});
	for (const std::string& input : inputs) {
		const std::string input_text = read_file(input);
		std::unique_ptr<Surface> output(renderer->render(input_text, width));

		if (!correct) {
			correct.reset(load_image(basedir + "correct.png", g_fs));
			if (!correct) {
				log("Could not load %s/correct.png.\n", basedir.c_str());
				return false;
			}
		}

		if (!compare_surfaces(correct.get(), output.get())) {
			log("Render output of %s does not compare equal to correct.png. Saved wrong.png.\n", input.c_str());

			std::unique_ptr<StreamWrite> sw(g_fs->OpenStreamWrite(basedir + "wrong.png"));
			if (!save_surface_to_png(output.get(), sw.get())) {
				std::cout << "Could not encode PNG." << std::endl;
			}
			return false;
		}

		std::unique_ptr<RT::IRefMap> ref_map(renderer->make_reference_map(input_text, width));
		for (const auto& sample : ref_map_samples) {
			BOOST_CHECK_EQUAL(sample.expected_text, ref_map->query(sample.x, sample.y));
		}
	}
	return true;
}

}  // namespace

BOOST_AUTO_TEST_CASE(b1206712) {
	// check that a single line of long chat message throws an error.
	BOOST_CHECK_THROW(compare_for_test(sar.renderer()), RT::WidthTooSmall);
}

BOOST_AUTO_TEST_CASE(br) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(bullet_point) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(escaped_text_simple) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_bold) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_color_blue) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_color_green) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_color_red) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_face) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_italic) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_ref) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_shadow) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_size_bigger) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_size_smaller) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_underline) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(font_various_attr) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(hline) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(hspace_dynamic1) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(hspace_dynamic2) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(hspace_dynamic_img) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(hspace_dynamic_text) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(hspace_fixed) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(img_simple) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(p_align) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(p_indent) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(p_spacing) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(p_valign_center) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(p_valign_default) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(p_valign_top) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(rt_bgcolor) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(rt_padding) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(rt_padding_allsites) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_autowidth_floatleftimg) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_background_img) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_fixedwidth_floatbothsides) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_fixedwidth_floatleft) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_fixedwidth_floatleftimg) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_fixedwidth_floatright) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_margin_bgclr) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_margin_bgimg) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_nonfloating_valign) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(sub_padding) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(table_like) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(text_linebreak) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(text_linebreak01) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(text_linebreak02) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(text_simple) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(text_simple_autowidth) {BOOST_CHECK(compare_for_test(sar.renderer()));}
BOOST_AUTO_TEST_CASE(vspace) {BOOST_CHECK(compare_for_test(sar.renderer()));}

BOOST_AUTO_TEST_SUITE_END();
