#include "graphic/graphic_functions.h"

#include <memory>

#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"

constexpr int kTextPadding = 48;

void draw_game_tip(RenderTarget& rt, const Recti& bounds, const std::string& text, unsigned opacity) {
	const Image& pic_background = load_safe_template_image("loadscreens/gametips.png");
	const int w = pic_background.width();
	const int h = pic_background.height();
	Vector2i pt(bounds.x + (bounds.w - w) / 2, bounds.y + (bounds.h - h) / 2);

	for (; opacity; --opacity) {
		rt.blit(pt, &pic_background);
	}

	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_game_tip(text), w - 2 * kTextPadding);
	pt = Vector2i(bounds.x + (bounds.w - rendered_text->width()) / 2,
	              bounds.y + (bounds.h - rendered_text->height()) / 2);
	rendered_text->draw(rt, pt);
}
