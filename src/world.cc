/*
 * Copyright (C) 2002 by Florian Bluemel
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
#include "os.h"
#include "world.h"
//#include "worldfiletypes.h"
#include "bob.h"
#include "myfile.h"
#include <cassert>

World::World(const char* file)
{
	name[0] = 0;
	author[0] = 0;
	bobCount = textureCount = animCount = resourceCount = terrainCount = 0;
	//
	Binary_file* wwf = new Binary_file();
	wwf->open(file, File::For::READ);
	if (wwf->get_state() == File::State::CLOSE)
		return;
	read_header(wwf);

/* TODO
 * these are skipped for now; mapeditor will need them
 */
	ResourceDesc res;
	for (uint i=0; i<resourceCount; i++)
		wwf->read(&res, sizeof(ResourceDesc));
	// same for terrain
	TerrainType terrain;
	for (uint j=0; j<terrainCount; j++)
		wwf->read(&terrain, sizeof(TerrainType));
/**/
	
	read_bobs(wwf);
	read_textures(wwf);
	read_anims(wwf);
}

void World::read_header(Binary_file* file)
{
	WorldFileHeader head;
	file->read(&head, sizeof(WorldFileHeader));
	// TODO: this should be handled gracefully
	assert(head.version == WLWF_VERSION);
	bobCount = head.bobs;
	textureCount = head.pictures;
	animCount = head.anims;
	terrainCount = head.terrains;
	resourceCount = head.resources;
	strcpy(name, head.name);
	strcpy(author, head.author);
}

void World::read_bobs(Binary_file* file)
{
	if (!bobCount)
		return;
	bob = new BobDesc[bobCount];
	for (uint i=0; i<bobCount; i++)
		file->read(&bob[i], sizeof(BobDesc));
}

void World::read_textures(Binary_file* file)
{
	if (!textureCount)
		return;
	texture = new Pic*[textureCount];
	for (uint i=0; i<textureCount; i++)
	{
		PictureInfo inf;
		file->read(&inf, sizeof(PictureInfo));
		ushort* pixel = new ushort[inf.height*inf.width];
		file->read(pixel, inf.height*inf.width*sizeof(ushort));
		texture[i] = new Pic();
		texture[i]->create(inf.width, inf.height, pixel);
		delete pixel;
	}
}

void World::read_anims(Binary_file* file)
{
	if (!animCount)
		return;
	anim = new Anim[animCount];
	for (uint i=0; i<animCount; i++)
	{
		file->read(&anim[i].pics, sizeof(uint));
		anim[i].pic = new uint[anim[i].pics];
		file->read(anim[i].pic, anim[i].pics*sizeof(uint));
	}
}

World::~World()
{
	delete[] texture;
	for (uint i=0; i<animCount; i++)
		delete anim[i].pic;
	delete anim;
}

Bob* World::create_bob(uint n)
{
	// create a new instance of bob n
	if (n < bobCount)
		return new Bob(&bob[n], this);
	return NULL;
}

Pic* World::get_texture(uint n)
{
	// return a new pointer to the texture
	if (n < textureCount)
		return texture[n];
	return NULL;
}

Anim* World::get_anim(uint n)
{
	// return a new pointer to the anim
	if (n < animCount)
		return &anim[n];
	return NULL;
}