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
#include <string.h>

/** World(const char* file)
  * Creates a new world from the file.
  */
World::World(const char* file)
{
	name[0] = 0;
	author[0] = 0;
	bobCount = textureCount = animCount = resourceCount = terrainCount = 0;
	bob = NULL;
	texture = NULL;
	anim = NULL;
	resource = NULL;
	terrain = NULL;
	//
	Binary_file wwf; 
	wwf.open(file, File::READ);
	if (wwf.get_state() == File::CLOSE)
		return;
	read_header(&wwf);

/* TODO
 * these are skipped for now; mapeditor will need them
 *
	ResourceDesc res;
	for (uint i=0; i<resourceCount; i++)
		wwf.read(&res, sizeof(ResourceDesc));
	// same for terrain
	TerrainType terrain;
	for (uint j=0; j<terrainCount; j++)
		wwf.read(&terrain, sizeof(TerrainType));
**/
	
	read_resources(&wwf);
	read_terrains(&wwf);
	read_bobs(&wwf);
	read_textures(&wwf);
	read_anims(&wwf);
}

/** void read_header(Binary_file*)
  * Reads the world header.
  */
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

/** void read_bobs(Binary_file*)
  * Reads the bob descriptions.
  */
void World::read_bobs(Binary_file* file)
{
	if (!bobCount)
		return;
	bob = new BobDesc[bobCount];
	for (uint i=0; i<bobCount; i++)
		file->read(&bob[i], sizeof(BobDesc));
}

/** void read_textures(Binary_file*)
  * Reads the textures.
  */
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

/** void read_anims(Binary_file*)
  * Reads the animations.
  */
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

/** void read_terrains(Binary_file*)
  * Reads the terrain types.
  */
void World::read_terrains(Binary_file* file)
{
	if (!terrainCount)
		return;
	terrain = new TerrainType[terrainCount];
	for (uint i=0; i<terrainCount; i++)
		file->read(&terrain[i], sizeof(TerrainType));
}

/** void read_resources(Binary_file*)
  * Reads the resource descriptions.
  */
void World::read_resources(Binary_file* file)
{
	if (!resourceCount)
		return;
	resource = new ResourceDesc[resourceCount];
	for (uint i=0; i<resourceCount; i++)
		file->read(&resource[i], sizeof(ResourceDesc));
}

/** ~World()
  * Armageddon.
  */
World::~World()
{
	if (texture)
		delete[] texture;	//?
	if (bob)
		delete[] bob;
	if (resource)
		delete[] resource;
	if (terrain)
		delete[] terrain;

	for (uint i=0; i<animCount; i++)
		delete anim[i].pic;
	if (anim)
		delete[] anim;
}

/** Bob* create_bob(uint n)
  * Returns a new instance of bob n.
  */
Bob* World::create_bob(uint n)
{
	if (n < bobCount)
		return new Bob(&bob[n], this);
	return NULL;
}

/** Pic* get_texture(uint n)
  * Returns texture n.
  */
Pic* World::get_texture(uint n)
{
	if (n < textureCount)
		return texture[n];
	return NULL;
}

/** Anim* get_anim(uint n)
  * Returns animation n.
  */
Anim* World::get_anim(uint n)
{
	if (n < animCount)
		return &anim[n];
	return NULL;
}

ResourceDesc* World::get_resource(uint n)
{
	return &resource[n];
}

TerrainType* World::get_terrain(uint n)
{
	return &terrain[n];
}

inline uint World::resources()
{
	return resourceCount;
}

inline uint World::terrains()
{
	return terrainCount;
}

inline uint World::bobs()
{
	return bobCount;
}
