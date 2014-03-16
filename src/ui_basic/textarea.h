/*
 * Copyright (C) 2002, 2006, 2008 by the Widelands Development Team
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

#ifndef UI_TEXTAREA_H
#define UI_TEXTAREA_H

#include "align.h"
#include "constants.h"
#include "graphic/font.h"
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
 * A multiline Textarea differs from a \ref Multiline_Textarea in that
 * the latter provides scrollbars.
 */
struct Textarea : public Panel {
	Textarea
		(Panel * parent,
		 int32_t x, int32_t y,
		 const std::string & text = std::string(),
		 Align align = Align_Left);
	Textarea
		(Panel * parent,
		 int32_t x, int32_t y, uint32_t w, uint32_t h,
		 Align align = Align_Left);
	Textarea
		(Panel *  const parent,
		 int32_t x, int32_t y, uint32_t w, uint32_t h,
		 const std::string & text,
		 Align align = Align_Left);
	Textarea
		(Panel * parent,
		 const std::string & text = std::string(),
		 Align align = Align_Left);

	void set_fixed_size(const std::string & text);
	void set_text(const std::string &);
	const std::string& get_text();

	// Drawing and event handlers
	void draw(RenderTarget &) override;

	void set_textstyle(const TextStyle & style);
	const TextStyle & get_textstyle() const {return m_textstyle;}

	void set_font(const std::string & name, int size, RGBColor clr);

protected:
	virtual void update_desired_size() override;

private:
	enum LayoutMode {
		AutoMove,
		Layouted,
		Static
	};

	void init();
	void collapse();
	void expand();

	LayoutMode m_layoutmode;
	std::string m_text;
	const Image* m_text_image;
	Align m_align;
	TextStyle m_textstyle;
};

}

#endif
