/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

// TODO(unknown): rename
#ifndef WL_GRAPHIC_FONT_HANDLER_H
#define WL_GRAPHIC_FONT_HANDLER_H

#include <memory>

#include "base/macros.h"
#include "graphic/image_cache.h"
#include "graphic/text/font_set.h"
#include "graphic/text/rendered_text.h"

namespace UI {

/**
 * Main class for string rendering. Manages the cache of pre-rendered strings.
 */
class IFontHandler {
public:
	IFontHandler() = default;
	virtual ~IFontHandler() {
	}

	/// Renders the given text into a set of images. The images are cached in a transient cache,
	/// so we share the ownership. Will throw on error.
	virtual std::shared_ptr<const UI::RenderedText> render(const std::string& text,
	                                                       uint16_t w = 0) = 0;

	/// Returns the font handler's current FontSet
	virtual UI::FontSet const* fontset() const = 0;

	/// Loads the FontSet for the currently active locale into the
	/// font handler. This needs to be called after the language of the
	/// game has changed.
	virtual void reinitialize_fontset(const std::string& locale) = 0;

	DISALLOW_COPY_AND_ASSIGN(IFontHandler);
};

/// Create a new FontHandler.
IFontHandler* create_fonthandler(ImageCache* image_cache, const std::string& locale);

extern IFontHandler* g_fh;
}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_FONT_HANDLER_H
