/*
 * Copyright (C) 2002 by Widelands Development Team
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

#ifndef __S__TEXTAREA_H
#define __S__TEXTAREA_H

/** 
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 */
class UITextarea : public UIPanel {
public:
	UITextarea(UIPanel *parent, int x, int y, std::string text, Align align = Align_Left);
	UITextarea(UIPanel *parent, int x, int y, int w, int h, std::string text,
			   Align align = Align_Left, bool multiline = false);
	~UITextarea();

	void set_text(std::string text);
	void set_align(Align align);

	// Drawing and event handlers
	void draw(RenderTarget* dst);

private:
	void collapse();
	void expand();

	std::string		m_text;
	Align				m_align;
	bool				m_multiline;
};

#endif // __S__TEXTAREA_H

