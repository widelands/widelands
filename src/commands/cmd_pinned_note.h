/*
 * Copyright (C) 2004-2025 by the Widelands Development Team
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

#ifndef WL_COMMANDS_CMD_PINNED_NOTE_H
#define WL_COMMANDS_CMD_PINNED_NOTE_H

#include "commands/command.h"
#include "graphic/color.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct CmdPinnedNote : PlayerCommand {
	CmdPinnedNote(const Time& t,
	              PlayerNumber p,
	              const std::string& text,
	              Coords pos,
	              const RGBColor& rgb,
	              bool del)
	   : PlayerCommand(t, p), text_(text), pos_(pos), rgb_(rgb), delete_(del) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kPinnedNote;
	}

	void execute(Game& game) override;

	explicit CmdPinnedNote(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdPinnedNote() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	std::string text_;
	Coords pos_;
	RGBColor rgb_;
	bool delete_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_PINNED_NOTE_H
