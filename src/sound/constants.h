/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_SOUND_CONSTANTS_H
#define WL_SOUND_CONSTANTS_H

#include <limits>

/* How important is it to play the effect even when others are running
 * already?
 *
 * \warning DO NOT CHANGE !! The values have meaning beyond just being numbers
 *
 * Value 0-127: probability between 0.0 and 1.0, only one instance can
 * be playing at any time
 *
 * Value 128-254: probability between 0.0 and 1.0, many instances can
 * be playing at any time
 *
 * Value 255: always play; unconditional
 */

/// Priorities lower than this one are illegal
constexpr uint16_t kFxPriorityLowest = 1;
/// This priority corresponds to 100%
constexpr uint16_t kFxMaximumPriority = 10000;

constexpr int32_t kStereoLeft = 0;
constexpr int32_t kStereoCenter = 128;
constexpr int32_t kStereoRight = 254;

using FxId = uint16_t;
constexpr FxId kNoSoundEffect = std::numeric_limits<uint16_t>::max();

/// Categorize sound effects and music to control their volume etc.
enum class SoundType { kUI, kMessage, kChat, kAmbient, kMusic };

#endif  // end of include guard: WL_SOUND_CONSTANTS_H
