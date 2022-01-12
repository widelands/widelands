/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_VISION_H
#define WL_LOGIC_VISION_H

namespace Widelands {

enum class HideOrRevealFieldMode { kReveal, kUnreveal, kHide, kUnexplore };

/// Simplified vision information reduced to 3 states.
/// Do not change the order! It is stored in savegames.
enum class VisibleState {
	kUnexplored = 0,      // Never seen yet
	kPreviouslySeen = 1,  // Previously seen
	kVisible = 2          // Currently visible
};

/// Extended vision information encoded as a number with some helper functions.
/// Used primarily in Player::Field.
/// The meaning of the numerical value:
///  0      if the player has never seen the node.
///  1      if the player does not currently see the node, but has seen
///         it previously.
///  2      if the player's buildings and workers do not currently see
///         the node, but it is visible to the player thanks to team
///         vision.
///  2+n    if the player's buildings and workers currently see the node,
///         where n is the number of objects that can see the node.
/// The 'override_' enum allows overriding the normal vision mechanism:
///  kNoOverride - no override
///  kHidden     - the field is always kUnexplored
///  kRevealed   - the field is always kVisible
struct Vision {
public:
	explicit Vision(const VisibleState vs) {
		switch (vs) {
		case VisibleState::kUnexplored:
			value_ = 0;
			break;
		case VisibleState::kPreviouslySeen:
			value_ = 1;
			break;
		case VisibleState::kVisible:
			// kVisible initializes to the "visible by teammates" state
			value_ = 2;
			break;
		}
		override_ = static_cast<uint8_t>(Override::kNoOverride);
	}
	Vision& operator=(const VisibleState vs) {
		switch (vs) {
		case VisibleState::kUnexplored:
			value_ = 0;
			break;
		case VisibleState::kPreviouslySeen:
			value_ = 1;
			break;
		case VisibleState::kVisible:
			// kVisible initializes to the "visible by teammates" state
			value_ = 2;
			break;
		}
		override_ = static_cast<uint8_t>(Override::kNoOverride);
		return *this;
	}

	uint16_t value() const {
		return value_;
	}
	operator VisibleState() const {
		switch (static_cast<Override>(override_)) {
		case Override::kHidden:
			return VisibleState::kUnexplored;
		case Override::kRevealed:
			return VisibleState::kVisible;
		default:
			switch (value_) {
			case 0:
				return VisibleState::kUnexplored;
			case 1:
				return VisibleState::kPreviouslySeen;
			default:
				return VisibleState::kVisible;
			}
		}
	}

	bool operator==(const Vision other) const {
		return value_ == other.value_ && override_ == other.override_;
	}

	bool operator!=(const Vision other) const {
		return value_ != other.value_ || override_ != other.override_;
	}

	bool is_explored() const {
		return value_ > 0;
	}
	bool is_visible() const {
		return value_ > 1;
	}
	bool is_seen_by_us() const {
		return seers() > 0 || is_revealed();
	}
	bool is_revealed() const {
		return static_cast<Override>(override_) == Override::kRevealed;
	}
	bool is_hidden() const {
		return static_cast<Override>(override_) == Override::kHidden;
	}
	uint16_t seers() const {
		return value_ > 2 ? (value_ - 2) : 0;
	}

	void increment_seers() {
		value_ = std::max(value_, uint16_t(2)) + 1;
	}
	void decrement_seers() {
		assert(seers() > 0);
		--value_;
	}
	void set_revealed(const bool reveal) {
		if (reveal == is_revealed()) {
			return;
		}
		if (reveal) {
			override_ = static_cast<uint8_t>(Override::kRevealed);
		} else {
			override_ = static_cast<uint8_t>(Override::kNoOverride);
		}
		value_ = std::max(value_, uint16_t(2));
	}
	void set_hidden(const bool hide) {
		if (hide == is_hidden()) {
			return;
		}
		if (hide) {
			override_ = static_cast<uint8_t>(Override::kHidden);
		} else {
			override_ = static_cast<uint8_t>(Override::kNoOverride);
		}
		if (seers() == 0) {
			value_ = 0;
		}
	}

private:
	enum class Override : uint8_t { kNoOverride = 0, kHidden = 1, kRevealed = 2 };
	uint16_t value_ : 14;
	// Not using the Override type for the variable because it causes a warning in GCC<8.4
	uint8_t override_ : 2;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_VISION_H
