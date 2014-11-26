/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXTURE_ATLAS_H
#define WL_GRAPHIC_TEXTURE_ATLAS_H

#include <memory>
#include <vector>

#include "graphic/sub_texture.h"
#include "graphic/texture.h"

// A 2d bin packer based on the blog post
// http://codeincomplete.com/posts/2011/5/7/bin_packing/.
class TextureAtlas {
public:
	// Create a TextureAtlas that is backed by one Texture of the given size.
	TextureAtlas(int width, int height);

	// NOCOM(#sirver): document
	void add(const Texture& texture);

	std::unique_ptr<Texture> pack(std::vector<std::unique_ptr<SubTexture>>* subtextures);

private:
	struct Node {
		Node(const Rect& init_r) : used(false), r(init_r) {
		}

		bool used;
		Rect r;
		std::unique_ptr<Node> right;
		std::unique_ptr<Node> down;

		void split(uint32_t w, uint32_t h);
	};

	struct Block {
		Block(int init_index, uint32_t width, uint32_t height, const Texture* init_texture)
		   : index(init_index), r(0, 0, width, height), texture(init_texture) {
		}

		int index;
		Rect r;
		const Texture* texture;
		Node* node;
	};

	static Node* find_node(Node* root, uint32_t w, uint32_t h);

	int next_index_;
	// Unpacked items.
	std::vector<Block> blocks_;

	DISALLOW_COPY_AND_ASSIGN(TextureAtlas);
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_ATLAS_H
