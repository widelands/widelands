/*
 * Copyright (C) 2022-2023 by the Widelands Development Team
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

#include "logic/map_objects/pinned_note.h"

#include <memory>

#include "graphic/rendertarget.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "wui/interactive_player.h"

namespace Widelands {

class PinnedNoteDescr : public BobDescr {
public:
	PinnedNoteDescr(char const* const init_name, char const* const init_descname)
	   : BobDescr(init_name,
	              init_descname,
	              MapObjectType::PINNED_NOTE,
	              MapObjectDescr::OwnerType::kTribe) {
	}

	[[nodiscard]] Bob& create_object() const override {
		return *new PinnedNote();
	}

private:
	DISALLOW_COPY_AND_ASSIGN(PinnedNoteDescr);
};

static const PinnedNoteDescr g_descr("pinned_note", "Pinned Note");

PinnedNote::PinnedNote() : Bob(g_descr) {
	owner_changed.connect([this](Player* old_owner, Player* new_owner) {
		if (old_owner != nullptr) {
			old_owner->unregister_pinned_note(this);
		}
		if (new_owner != nullptr) {
			new_owner->register_pinned_note(this);
		}
	});
}

// static
PinnedNote& PinnedNote::create(EditorGameBase& egbase,
                               Player& owner,
                               Coords pos,
                               const std::string& text,
                               const RGBColor& rgb) {
	PinnedNote& note = *new PinnedNote();
	note.set_text(text);
	note.set_rgb(rgb);
	note.set_position(egbase, pos);
	note.init(egbase);
	note.set_owner(&owner);
	return note;
}

void PinnedNote::init_auto_task(Game& game) {
	if (get_position().field->get_immovable() != nullptr &&
	    get_position().field->get_immovable()->descr().type() >= MapObjectType::BUILDING) {
		/* If the player builds a building here, move one field out of the way. */
		FCoords new_pos = game.map().br_n(get_position());
		Notifications::publish(NotePinnedNoteMoved(owner().player_number(), get_position(), new_pos));
		set_position(game, new_pos);
	}

	start_task_idle(game, 0, 200);
}

void PinnedNote::draw(const EditorGameBase& egbase,
                      const InfoToDraw& info_to_draw,
                      const Vector2f& field_on_dst,
                      const Coords& coords,
                      float scale,
                      RenderTarget* dst) const {
	if (get_owner() == nullptr) {
		// Note not yet fully initialized, can happen during creation
		return;
	}

	if (egbase.is_game()) {
		const InteractivePlayer* ipl = dynamic_cast<const Game&>(egbase).get_ipl();
		if (ipl != nullptr) {
			const Player& p = ipl->player();
			if (&p != &owner() &&
			    (owner().team_number() == 0 || owner().team_number() != p.team_number())) {
				/* Notes are invisible to enemy players. */
				return;
			}
		}
	}

	dst->blit_animation(field_on_dst, coords, scale, owner().tribe().pinned_note_animation(),
	                    egbase.get_gametime(), &rgb_);
	do_draw_info(info_to_draw, text_, std::string(), field_on_dst, scale, dst);
}

constexpr uint8_t kCurrentPacketVersion = 1;

MapObject::Loader* PinnedNote::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version == 1) {
			PinnedNote& note = *new PinnedNote;

			note.set_text(fr.string());
			RGBColor c;
			c.r = fr.unsigned_8();
			c.g = fr.unsigned_8();
			c.b = fr.unsigned_8();
			note.set_rgb(c);

			loader->init(egbase, mol, note);
			loader->load(fr);
		} else {
			throw UnhandledVersionError("PinnedNote", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading pinned note: %s", e.what());
	}

	return loader.release();
}

void PinnedNote::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderPinnedNote);
	fw.unsigned_8(kCurrentPacketVersion);
	fw.string(text_);
	fw.unsigned_8(rgb_.r);
	fw.unsigned_8(rgb_.g);
	fw.unsigned_8(rgb_.b);
	Bob::save(egbase, mos, fw);
}

}  // namespace Widelands
