/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#include "wui/edge_overlay_manager.h"

EdgeOverlayManager::EdgeOverlayManager() : m_current_overlay_id(0) {
}

EdgeOverlayManager::OverlayId EdgeOverlayManager::next_overlay_id() {
	++m_current_overlay_id;
	return m_current_overlay_id;
}

void EdgeOverlayManager::register_overlay
	(Widelands::Coords const c, uint8_t const where, OverlayId const overlay_id)
{
	const RegisteredRoadOverlays overlay = {overlay_id, where};
	RegisteredRoadOverlaysMap::iterator it = m_overlays.find(c);
	if (it == m_overlays.end())
		m_overlays.insert
			(std::pair<const Widelands::Coords,
			 RegisteredRoadOverlays>(c, overlay));
	else
		it->second = overlay;
}

void EdgeOverlayManager::remove_overlay(const Widelands::Coords c) {
	const RegisteredRoadOverlaysMap::iterator it = m_overlays.find(c);
	if (it != m_overlays.end())
		m_overlays.erase(it);
}

void EdgeOverlayManager::remove_overlay(OverlayId const overlay_id) {
	RegisteredRoadOverlaysMap::iterator it = m_overlays.begin();
	const RegisteredRoadOverlaysMap::const_iterator end =
		m_overlays.end();
	while (it != end)
		if (it->second.overlay_id == overlay_id)
			m_overlays.erase(it++); //  Necessary!
		else
			++it;
}

uint8_t EdgeOverlayManager::get_overlay(const Widelands::Coords c) const {
	RegisteredRoadOverlaysMap::const_iterator const it = m_overlays.find(c);
	if (it != m_overlays.end())
		return it->second.where;
	return 0;
}
