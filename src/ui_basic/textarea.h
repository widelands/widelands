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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_UI_BASIC_TEXTAREA_H
#define WL_UI_BASIC_TEXTAREA_H

#include <memory>

#include "graphic/align.h"
#include "graphic/styles/font_style.h"
#include "graphic/text/rendered_text.h"
#include "ui_basic/panel.h"

namespace UI {

/**
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed.
 *
 * Textareas can operate in auto-move mode or in layouted mode.
 *
 * In auto-move mode, which is selected by constructors that take x/y coordinates
 * as parameters, the given (x,y) is used as the anchor for the text.
 * The panel automatically changes its size and position so that the
 * given (x,y) always stay the anchor point. This is incompatible with
 * using the Textarea in a layouted situation, e.g. inside \ref Box.
 *
 * In layouted mode, which is selected by the constructor that does not
 * take coordinates, the textarea simply sets its desired size
 * appropriately for the contained text.
 *
 * Finally, there is static mode, which does not change desired or actual
 * size in any way based on the text.
 *
 * A multiline Textarea differs from a \ref MultilineTextarea in that
 * the latter provides scrollbars.
 */
struct Textarea : public Panel {
public:
	explicit Textarea(Panel* const parent,
	                  PanelStyle,
	                  FontStyle,
	                  int32_t x,
	                  int32_t y,
	                  uint32_t w,
	                  uint32_t h,
	                  const std::string& text = std::string(),
	                  Align align = UI::Align::kLeft);
	explicit Textarea(Panel* parent,
	                  PanelStyle,
	                  FontStyle,
	                  const std::string& text = std::string(),
	                  Align align = UI::Align::kLeft);

	/**
	 * If fixed_width > 0, the Textarea will not change its width.
	 * Use this if you need a Textarea that keeps changing its contents, but you don't want the
	 * surrounding elements to shift, e.g. in a Box.
	 */
	void set_fixed_width(int w);

	void set_text(const std::string&);
	const std::string& get_text();

	// Drawing and event handlers
	void draw(RenderTarget&) override;
	void update_template() override;

	void set_style(FontStyle);
	void set_style_override(const FontStyleInfo&);
	void set_font_scale(float scale);

protected:
	void update_desired_size() override;

private:
	enum class LayoutMode { AutoMove, Layouted };

	Textarea(Panel* const parent,
	         PanelStyle,
	         FontStyle,
	         int32_t x,
	         int32_t y,
	         uint32_t w,
	         uint32_t h,
	         const std::string& text,
	         Align align,
	         LayoutMode layout_mode);

	void collapse();
	void expand();
	void update();

	const LayoutMode layoutmode_;
	const Align align_;

	std::string text_;
	std::shared_ptr<const UI::RenderedText> rendered_text_;

	FontStyle font_style_;
	const FontStyleInfo* font_style_override_;
	const FontStyleInfo& font_style() const;

	float font_scale_;
	int fixed_width_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_TEXTAREA_H
