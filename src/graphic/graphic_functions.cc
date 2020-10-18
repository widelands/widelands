#include "graphic/graphic_functions.h"

#include <memory>

#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"

constexpr int kTextPadding = 48;

void draw_game_tip(const std::string& text, unsigned opacity) {
	RenderTarget& rt = *g_gr->get_render_target();

	const Image* pic_background =
	   g_image_cache->get(std::string(kTemplateDir) + "loadscreens/gametips.png");
	const int w = pic_background->width();
	const int h = pic_background->height();
	Vector2i pt((g_gr->get_xres() - w) / 2, (g_gr->get_yres() - h) / 2);

	for (; opacity; --opacity) {
		rt.blit(pt, pic_background);
	}

	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_game_tip(text), w - 2 * kTextPadding);
	pt = Vector2i((g_gr->get_xres() - rendered_text->width()) / 2,
	              (g_gr->get_yres() - rendered_text->height()) / 2);
	rendered_text->draw(rt, pt);
}
