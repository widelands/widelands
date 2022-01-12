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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/texture_atlas.h"

#include <algorithm>
#include <cassert>
#include <memory>

#include "base/wexception.h"

namespace {

// This padding will be applied to the left and bottom of each block to
// separate them during blitting if OpenGL decides to be a jerk about where to
// sample.
constexpr int kPadding = 1;

}  // namespace

TextureAtlas::Node::Node(const Recti& init_r) : used(false), r(init_r) {
}

void TextureAtlas::Node::split(int item_w, int item_h) {
	assert(!used);

	down.reset(new Node(Recti(r.x, r.y + item_h, r.w, r.h - item_h)));
	right.reset(new Node(Recti(r.x + item_w, r.y, r.w - item_w, item_h)));
	used = true;

	// Note: we do not change the size of the root. It is not needed
	// for the remaining algorithm, but we use it to remember the
	// size of the full canvas.
}

TextureAtlas::TextureAtlas() : next_index_(0) {
}

void TextureAtlas::add(const Image& texture) {
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

std::unique_ptr<Texture> TextureAtlas::pack_as_many_as_possible(
   const int max_dimension, const int texture_atlas_index, std::vector<PackedTexture>* pack_info) {

	std::unique_ptr<Node> root(new Node(Recti(0, 0, blocks_.begin()->texture->width() + kPadding,
	                                          blocks_.begin()->texture->height() + kPadding)));

	const auto grow_right = [&root](int delta_w) {
		std::unique_ptr<Node> new_root(new Node(Recti(0, 0, root->r.w + delta_w, root->r.h)));
		new_root->used = true;
		new_root->right.reset(new Node(Recti(root->r.w, 0, delta_w, root->r.h)));
		new_root->down = std::move(root);
		root = std::move(new_root);
	};

	const auto grow_down = [&root](int delta_h) {
		std::unique_ptr<Node> new_root(new Node(Recti(0, 0, root->r.w, root->r.h + delta_h)));
		new_root->used = true;
		new_root->down.reset(new Node(Recti(0, root->r.h, root->r.w, delta_h)));
		new_root->right = std::move(root);
		root = std::move(new_root);
	};

	std::vector<Block> packed, not_packed;
	for (Block& block : blocks_) {
		const int block_width = block.texture->width() + kPadding;
		const int block_height = block.texture->height() + kPadding;

		Node* fitting_node = find_node(root.get(), block_width, block_height);
		if (fitting_node == nullptr) {
			// Atlas is not big enough to contain this. Grow it and try again.
			bool can_grow_down =
			   (block_width <= root->r.w) && (block_height + root->r.h < max_dimension);
			bool can_grow_right =
			   (block_height <= root->r.h) && (block_width + root->r.w < max_dimension);

			// Attempt to keep the texture square-ish.
			bool should_grow_right = can_grow_right && (root->r.h >= root->r.w + block_width);
			bool should_grow_down = can_grow_down && (root->r.w >= root->r.h + block_height);

			if (should_grow_right) {  // NOLINT
				grow_right(block_width);
			} else if (should_grow_down) {  // NOLINT
				grow_down(block_height);
			} else if (can_grow_right) {
				grow_right(block_width);
			} else if (can_grow_down) {
				grow_down(block_height);
			}
			fitting_node = find_node(root.get(), block_width, block_height);
		}
		if (fitting_node) {
			fitting_node->split(block_width, block_height);
			block.node = fitting_node;
			packed.push_back(block);
		} else {
			not_packed.push_back(block);
		}
	}

	std::unique_ptr<Texture> texture_atlas(new Texture(root->r.w, root->r.h));
	texture_atlas->fill_rect(Rectf(0.f, 0.f, root->r.w, root->r.h), RGBAColor(0, 0, 0, 0));

	const auto packed_texture_id = texture_atlas->blit_data().texture_id;
	for (Block& block : packed) {
		texture_atlas->blit(
		   Rectf(block.node->r.x, block.node->r.y, block.texture->width(), block.texture->height()),
		   *block.texture, Rectf(0.f, 0.f, block.texture->width(), block.texture->height()), 1.,
		   BlendMode::Copy);

		pack_info->emplace_back(PackedTexture(
		   texture_atlas_index, block.index,
		   std::unique_ptr<Texture>(new Texture(
		      packed_texture_id,
		      Recti(block.node->r.origin(), block.texture->width(), block.texture->height()),
		      root->r.w, root->r.h))));
	}
	blocks_ = not_packed;
	return texture_atlas;
}

void TextureAtlas::pack(const int max_dimension,
                        std::vector<std::unique_ptr<Texture>>* texture_atlases,
                        std::vector<PackedTexture>* pack_info) {
	if (blocks_.empty()) {
		throw wexception("Called pack() without blocks.");
	}

	// Sort blocks by their biggest side length. This heuristically gives the
	// best packing.
	std::sort(blocks_.begin(), blocks_.end(), [](const Block& i, const Block& j) {
		return std::max(i.texture->width(), i.texture->height()) >
		       std::max(j.texture->width(), j.texture->height());
	});

	while (!blocks_.empty()) {
		texture_atlases->emplace_back(
		   pack_as_many_as_possible(max_dimension, texture_atlases->size(), pack_info));
	}

	// Sort pack info by index so that they come back in the correct ordering.
	std::sort(pack_info->begin(), pack_info->end(),
	          [](const PackedTexture& i, const PackedTexture& j) { return i.index_ < j.index_; });
}
