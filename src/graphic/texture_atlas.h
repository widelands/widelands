/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXTURE_ATLAS_H
#define WL_GRAPHIC_TEXTURE_ATLAS_H

#include <memory>

#include "base/macros.h"
#include "graphic/texture.h"

// A 2d bin packer based on the blog post
// https://codeincomplete.com/posts/bin-packing/.
class TextureAtlas {
public:
	struct PackedTexture {
		PackedTexture() = default;

		// The index of the returned texture atlas that contains this image.
		int texture_atlas = -1;

		// The newly packed texture.
		std::unique_ptr<Texture> texture{nullptr};

	private:
		friend class TextureAtlas;

		PackedTexture(int init_texture_atlas, int index, std::unique_ptr<Texture> init_texture)
		   : texture_atlas(init_texture_atlas), texture(std::move(init_texture)), index_(index) {
		}

		// The position the images was 'add'()ed into the packing queue. Purely internal.
		int index_ = -1;
	};

	TextureAtlas() = default;

	// Add 'texture' as one of the textures to be packed. Ownership is
	// not taken, but 'texture' must be valid until pack() has been
	// called.
	void add(const Image& texture);

	// Packs the textures into as many texture atlases as needed, so that none
	// of them will be larger than 'max_dimension' x 'max_dimension'. The
	// returned 'textures' contains the individual sub textures (that do not own
	// their memory) in the order they have been added by 'add'.
	void pack(int max_dimension,
	          std::vector<std::unique_ptr<Texture>>* texture_atlases,
	          std::vector<PackedTexture>* pack_info);

private:
	struct Node {
		explicit Node(const Recti& init_r);
		void split(int w, int h);

		bool used{false};
		Recti r;
		std::unique_ptr<Node> right;
		std::unique_ptr<Node> down;

		DISALLOW_COPY_AND_ASSIGN(Node);
	};

	struct Block {
		Block(int init_index, const Image* init_texture) : index(init_index), texture(init_texture) {
		}

		// The index in the order the blocks have been added.
		int index;
		const Image* texture;
		Node* node{nullptr};

		// True if this block has already been packed into a texture atlas.
		bool done{false};
	};

	// Packs as many blocks from 'blocks_' that still have done = false into a
	// fresh texture atlas that will not grow bigger than 'max_size' x
	// 'max_size'.
	std::unique_ptr<Texture> pack_as_many_as_possible(int max_dimension,
	                                                  int texture_atlas_index,
	                                                  std::vector<PackedTexture>* pack_info);
	static Node* find_node(Node* node, int w, int h);

	int next_index_{0};

	// Unpacked items.
	std::vector<Block> blocks_;

	DISALLOW_COPY_AND_ASSIGN(TextureAtlas);
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_ATLAS_H
