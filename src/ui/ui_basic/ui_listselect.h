/*
 * Copyright (C) 2002 by the Widelands Development Team
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


#ifndef __S__LISTSELECT_H
#define __S__LISTSELECT_H

/** 
 * This class defines a list-select box.
 */
class Listselect : public Panel {
public:
	Listselect(Panel *parent, int x, int y, uint w, uint h, Align align = Align_Left);
	~Listselect();

	UISignal1<int> selected;

	void clear();
	void add_entry(const char *name, void *value);

	void set_align(Align align);

	void select(int i);
	inline void *get_selection() {
		if (m_selection < 0) return 0;
		return m_entries[m_selection]->value;
	}

	int get_lineheight();
	inline uint get_eff_w() { return get_w(); }

	// Drawing and event handling
	void draw(RenderTarget* dst);
	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	void set_scrollpos(int pos);

private:
	static const int ms_darken_value=-20;

   struct Entry {
		void*		value;
		char		name[1];
	};

	Align						m_align;
	std::vector<Entry*>	m_entries;
	Scrollbar*				m_scrollbar;
	int						m_scrollpos;	// in pixels
	int						m_selection;	// -1 when nothing is selected
};

#endif
