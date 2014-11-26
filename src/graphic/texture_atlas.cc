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

#include "base/log.h"
#include "graphic/texture_atlas.h"

#include <memory>

TextureAtlas::TextureAtlas(int, int) :
	next_index_(0)
{
}

void TextureAtlas::add(const Texture& texture) {
	blocks_.emplace_back(next_index_++, texture.width(), texture.height(), &texture);
}

// static
TextureAtlas::Node* TextureAtlas::find_node(Node* root, uint32_t w, uint32_t h) {
	if (root->used) {
		Node* child_node = find_node(root->right.get(), w, h);
		if (child_node != nullptr) {
			return child_node;
		}
		return find_node(root->down.get(), w, h);
	}

	if (w <= root->r.w && h <= root->r.h) {
		return root;
	}

	// NOCOM(#sirver): this is fatal.
	return nullptr;
}

void TextureAtlas::Node::split(uint32_t item_w, uint32_t item_h) {
	used = true;
	down.reset(new Node(Rect(r.x, r.y + item_h, r.w, r.h - item_h)));
	right.reset(new Node(Rect(r.x + item_w, r.y, r.w - item_w, r.h)));
}

std::unique_ptr<Texture> TextureAtlas::pack(std::vector<std::unique_ptr<SubTexture>>* subtextures) {

	std::sort(blocks_.begin(), blocks_.end(), [](const Block& i, const Block& j) {
		return std::max(i.r.w, i.r.h) > std::max(j.r.w, j.r.h);
	});

	log("#sirver blocks_.size(): %d\n", blocks_.size());
	// NOCOM(#sirver): make this grow.
	uint32_t current_w = 1024;
	uint32_t current_h = 1024;

	Node root(Rect(0, 0, current_w, current_h));

	// NOCOM(#sirver): make sure nodes contains something.
	for (Block& block : blocks_) {
		Node* fitting_node = find_node(&root, block.r.w, block.r.h);
		fitting_node->split(block.r.w, block.r.h);
		block.node = fitting_node;
	}

	// NOCOM(#sirver): block only needs w and h. And that can be methos forwards from the texture.
	std::unique_ptr<Texture> packed_texture(new Texture(current_w, current_h));
	packed_texture->fill_rect(Rect(0, 0, current_w, current_h), RGBAColor(0, 0, 0, 0));

	for (Block& block : blocks_) {
		packed_texture->blit(block.node->r.top_left(),
		                     block.texture,
		                     Rect(0, 0, block.texture->width(), block.texture->height()));
	}
	return packed_texture;
}
