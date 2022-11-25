/*
 * Copyright (C) 2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_PINNED_NOTE_H
#define WL_LOGIC_MAP_OBJECTS_PINNED_NOTE_H

#include "logic/map_objects/bob.h"

namespace Widelands {

struct NotePinnedNoteMoved {
	CAN_BE_SENT_AS_NOTE(NoteId::PinnedNoteMoved)

	PlayerNumber player;
	FCoords old_pos;
	FCoords new_pos;

	NotePinnedNoteMoved(PlayerNumber p, FCoords o, FCoords n) : player(p), old_pos(o), new_pos(n) {
	}
};

class PinnedNote : public Bob {
public:
	PinnedNote();
	static PinnedNote& create(EditorGameBase& egbase,
	                          Player& owner,
	                          Coords pos,
	                          const std::string& text,
	                          const RGBColor& rgb);

	void draw(const EditorGameBase&,
	          const InfoToDraw& info_to_draw,
	          const Vector2f& field_on_dst,
	          const Coords& coords,
	          float scale,
	          RenderTarget* dst) const override;
	void init_auto_task(Game& game) override;

	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;
	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);

	void set_text(const std::string& text) {
		text_ = text;
	}
	const std::string& get_text() const {
		return text_;
	}

	void set_rgb(const RGBColor rgb) {
		rgb_ = rgb;
	}
	const RGBColor& get_rgb() const {
		return rgb_;
	}

private:
	std::string text_;
	RGBColor rgb_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_PINNED_NOTE_H
