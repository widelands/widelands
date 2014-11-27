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

#include "graphic/texture_atlas.h"

#include <cassert>
#include <memory>

#include "base/log.h"

TextureAtlas::Node::Node(const Rect& init_r) : used(false), r(init_r) {
	// log("#sirver init_r.x: %d,init_r.y: %d,init_r.w: %d,init_r.h: %d\n",
		 // init_r.x,
		 // init_r.y,
		 // init_r.w,
		 // init_r.h);
}
void TextureAtlas::Node::split(int item_w, int item_h) {
	assert(!used);

	down.reset(new Node(Rect(r.x, r.y + item_h, r.w, r.h - item_h)));
	right.reset(new Node(Rect(r.x + item_w, r.y, r.w - item_w, item_h)));
	used = true;

	// Note: we do not change the size of the root. It is not needed
	// for the remaining algorithm, but we use it to remember the
	// size of the full canvas.
}


TextureAtlas::TextureAtlas(int, int) :
	next_index_(0)
{
}

void TextureAtlas::add(const Texture& texture) {
	blocks_.emplace_back(next_index_++, &texture);
}

// static
TextureAtlas::Node* TextureAtlas::find_node(Node* node, int w, int h) {
	if (node->used) {
		Node* child_node = find_node(node->right.get(), w, h);
		if (child_node != nullptr) {
			return child_node;
		}
		return find_node(node->down.get(), w, h);
	}
	assert(!node->used);

	if ((w <= node->r.w) && (h <= node->r.h)) {
		return node;
	}

	return nullptr;
}

std::unique_ptr<Texture> TextureAtlas::pack(std::vector<std::unique_ptr<Texture>>* textures) {

	std::sort(blocks_.begin(), blocks_.end(), [](const Block& i, const Block& j) {
		return std::max(i.texture->width(), i.texture->height()) >
		       std::max(j.texture->width(), j.texture->height());
	});

	// NOCOM(#sirver): make this grow.
	int current_w = 1024;
	int current_h = 1024;

	Node root(Rect(0, 0, current_w, current_h));

	// NOCOM(#sirver): make sure nodes contains something.
	for (Block& block : blocks_) {
		Node* fitting_node = find_node(&root, block.texture->width(), block.texture->height());
		fitting_node->split(block.texture->width(), block.texture->height());
		log("#sirver    fitting_node->x: %d,fitting_node->y: %d,fitting_node->w: %d,fitting_node->h: "
		    "%d\n",
		    fitting_node->r.x,
		    fitting_node->r.y,
		    fitting_node->r.w,
		    fitting_node->r.h);
		block.node = fitting_node;
	}

	std::unique_ptr<Texture> packed_texture(new Texture(current_w, current_h));
	packed_texture->fill_rect(Rect(0, 0, current_w, current_h), RGBAColor(0, 0, 0, 0));

	// NOCOM(#sirver): sort block by index.

	for (Block& block : blocks_) {
		packed_texture->blit(block.node->r.top_left(),
		                     block.texture,
		                     Rect(0, 0, block.texture->width(), block.texture->height()));
		textures->emplace_back(new Texture(
		   packed_texture->get_gl_texture(),
		   Rect(block.node->r.top_left(), block.texture->width(), block.texture->height()),
		   current_w,
		   current_h));
	}
	return packed_texture;
}
