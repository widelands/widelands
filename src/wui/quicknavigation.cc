/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#include "wui/quicknavigation.h"

#include "logic/editor_game_base.h"
#include "wlapplication.h"
#include "wui/mapviewpixelfunctions.h"

static const uint32_t MaxHistorySize = 32;

QuickNavigation::QuickNavigation
	(const Widelands::Editor_Game_Base & egbase,
	 uint32_t screenwidth, uint32_t screenheight)
: m_egbase(egbase)
{
	m_screenwidth = screenwidth;
	m_screenheight = screenheight;

	m_havefirst = false;
	m_update = true;
	m_history_index = 0;
}

void QuickNavigation::set_setview(const QuickNavigation::SetViewFn & fn)
{
	m_setview = fn;
}

void QuickNavigation::setview(Point where)
{
	m_update = false;
	m_setview(where);
	m_update = true;
}

void QuickNavigation::view_changed(Point newpos, bool jump)
{
	if (m_havefirst && m_update) {
		if (!jump) {
			Point delta =
				MapviewPixelFunctions::calc_pix_difference
					(m_egbase.map(), newpos, m_anchor);

			if
				(static_cast<uint32_t>(abs(delta.x)) > m_screenwidth ||
			    	 static_cast<uint32_t>(abs(delta.y)) > m_screenheight)
				jump = true;
		}

		if (jump) {
			if (m_history_index < m_history.size())
				m_history.erase
					(m_history.begin() + m_history_index,
					 m_history.end());
			m_history.push_back(m_current);
			if (m_history.size() > MaxHistorySize)
				m_history.erase
					(m_history.begin(),
					 m_history.end() - MaxHistorySize);
			m_history_index = m_history.size();
		}
	}

	if (jump || !m_havefirst) {
		m_anchor = newpos;
	}

	m_current = newpos;
	m_havefirst = true;
}

bool QuickNavigation::handle_key(bool down, SDL_keysym key)
{
	if (!m_havefirst)
		return false;
	if (!down)
		return false;

	if (key.sym >= SDLK_0 && key.sym <= SDLK_9) {
		unsigned int which = key.sym - SDLK_0;
		assert(which < 10);

		bool ctrl =
			WLApplication::get()->get_key_state(SDLK_LCTRL) ||
			WLApplication::get()->get_key_state(SDLK_RCTRL);
		if (ctrl) {
			m_landmarks[which].point = m_current;
			m_landmarks[which].set = true;
		} else {
			if (m_landmarks[which].set)
				m_setview(m_landmarks[which].point);
		}
		return true;
	}

	if (key.sym == SDLK_COMMA) {
		if (m_history_index > 0) {
			if (m_history_index >= m_history.size()) {
				m_history.push_back(m_current);
			}
			m_history_index--;
			setview(m_history[m_history_index]);
		}
		return true;
	}

	if (key.sym == SDLK_PERIOD) {
		if (m_history_index + 1 < m_history.size()) {
			m_history_index++;
			setview(m_history[m_history_index]);
		}
		return true;
	}

	return false;
}
