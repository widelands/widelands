/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "map.h"
#include "worlddata.h"
#include "game.h"

/** Map::Pathfield
 *
 * Used in pathfinding. For better encapsulation, pathfinding structures
 * are seperate from normal fields
 *
 * Costs are in milliseconds to walk.the route.
 *
 * Note: member sizes chosen so that we get a 16byte (=nicely aligned) 
 * structure
 */
struct Map::Pathfield {
	int		heap_index;		// index of this field in heap, for backlinking
	int		real_cost;		// true cost up to this field
	int		estim_cost;		// estimated cost till goal
	ushort	cycle;
	uchar		backlink;		// how we got here (Map_Object::WALK_*)
	
	inline int cost() { return real_cost + estim_cost; }
};

/** class Map
 *
 * This really identifies a map like it is in the game
 *
 * DEPENDS: class File
 * 			class g_fileloc
 */

/** Map::Map(void)
 *
 * inits
 */
Map::Map(void)
{
	w=0;
	fields=0;
   starting_pos=0;
	m_pathcycle = 0;
	m_pathfields = 0;
}

/** Map::~Map(void)
 *
 * cleanups
 */
Map::~Map(void)
{
   if(fields) {
      // WARNING: if Field has to free something, we have to do
      // it here manually!!!
      free(fields);
   }
	if (m_pathfields)
		free(m_pathfields);

   if(starting_pos) {
      free(starting_pos);
   }

   if(w) delete w;
}

/** void Map::set_size(uint w, uint h)
 *
 * This sets the size of the current map
 *
 * *** PRIAVTE FUNC ****
 *
 * Args:	w, h	size of map
 * Returns: Nothing
 */
void Map::set_size(uint w, uint h) {
   hd.width=w;
   hd.height=h;

   if(fields) {
      fields = (Field*) realloc(fields, sizeof(Field)*hd.height*hd.width);
   }
   else fields = (Field*) malloc(sizeof(Field)*hd.height*hd.width);

	if (m_pathfields)
		free(m_pathfields);
	m_pathfields = (Pathfield*)malloc(sizeof(Pathfield)*hd.height*hd.width);
	memset(m_pathfields, 0, sizeof(Pathfield)*hd.height*hd.width);
}

/** int Map::load_wlmf(const char* file)
 *
 * this loads a given file as a widelands map file
 *
 * ***** PRIVATE FUNC ******
 *
 * Args: 	file		filename to read
 * Returns: RET_OK or RET_FAILED
 */

// TODO: This function is not working!!
//  Well, I wouldn't bother about that as we don't even have a map editor... -- Nicolai
#if 0
int Map::load_wlmf(const char* file, Game *game)
{
	FileRead f

   f.Load(g_fs, file);

   // read header:
	memcpy(&hd, f.Data(sizeof(hd)), sizeof(hd));

   // check version
   if(WLMF_VERSIONMAJOR(hd.version) > WLMF_VERSIONMAJOR(WLMF_VERSION)) {
      return ERR_FAILED;
   }
   if(WLMF_VERSIONMAJOR(hd.version) == WLMF_VERSIONMAJOR(WLMF_VERSION)) {
      if(WLMF_VERSIONMINOR(hd.version) > WLMF_VERSIONMINOR(WLMF_VERSION)) {
         return ERR_FAILED;
      }
   }

   // ignore the player descriptions, probably the user has chnanged them.
   // as long as the game knows how many players are around, everything is ok
   PlayerDescr pl;
   for(uint i=0; i<hd.nplayers; i++) {
      f.Data(sizeof(PlayerDescr));
   }

   set_size(hd.width, hd.height);


   // now, read in the fields, one at a time and init the map
   FieldDescr *fd;
   uint y;
   Terrain_Descr *td, *tr;
   for(y=0; y<hd.height; y++) {
      for(uint x=0; x<hd.width; x++) {
			fd = (FieldDescr *)f.Data(sizeof(FieldDescr));

         // TEMP
         tr=w->get_terrain(fd->tex_r);
         if(!tr) {
            //cerr << "Texture number " << fd.tex_r << " not found in file. Defaults to 0" << endl;
            tr=w->get_terrain(0);
            assert(0); // we should never be here!  
         }
         td=w->get_terrain(fd->tex_d);
         if(!td) {
            // cerr << "Texture number " << fd.tex_d << " not found in file. Defaults to 0" << endl;
            td=w->get_terrain(0);
            assert(0); // we should never be here!  
         }
         // TEMP end

         Field* f=get_field(x,y);
         f->set_height((uchar)fd->height);
         f->set_terrainr(tr);
         f->set_terraind(td);
			f->objects = 0;
      }
   }

   return RET_OK;
}
#endif

/** 
 * This functions load a map header, for previewing maps.
 *
 * Returns RET_OK if the map is invalid
 */
int Map::load_map_header(const char* file)
{
   int ret=RET_OK;

   if(!strcasecmp(file+(strlen(file)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
   {
      // It ends like a wide lands map file. try to load
      // it as such
      // TODO: do this
      // ret = load_wlmf(file, q);
		return ERR_FAILED;
   }
   else if(!strcasecmp(file+(strlen(file)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX))
   {
      // it is a S2 Map file. load it as such
		try {
      	ret = load_s2mf_header(file);
		} catch(std::exception &e) {
			cerr << "Problem loading map " << file << ": " << e.what() << endl;
			ret = ERR_FAILED;
		}
   }
   else
   {
      assert(0);
      return ERR_FAILED;
   }

   return ret;
}

/** int Map::load_map(const char* file)
 *
 * This loads a complete map from a file
 *
 * Args: file	filename to read
 * Returns: RET_OK or ERR_FAILED
 */
int Map::load_map(const char* file, Game* g)
{
	try
	{
		if(!strcasecmp(file+(strlen(file)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
		{
			// It ends like a wide lands map file. try to load
			// it as such
			//ret = load_wlmf(file, g);
			throw wexception("WLMF not supported");
		}
		else if(!strcasecmp(file+(strlen(file)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX))
		{
			// it is a S2 Map file. load it as such
			load_s2mf(file, g);
		}
		else
			throw wexception("Unknown map file format");

		// Post process the map in the necessary two passes
		Field *f;
		uint y;
		for(y=0; y<hd.height; y++) {
			for(uint x=0; x<hd.width; x++) {
				f = get_field(x, y);
				recalc_brightness(x, y, f);
				recalc_fieldcaps_pass1(x, y, f);
			}
		}

		for(y=0; y<hd.height; y++) {
			for(uint x=0; x<hd.width; x++) {
				f = get_field(x, y);
				recalc_fieldcaps_pass2(x, y, f);
			}
		}
	}
	catch(std::exception &e) {
		cerr << "Error loading map " << file << ": " << e.what() << endl;
		return ERR_FAILED;
	}
	
   return RET_OK;
}

/** Map::find_objects(int x, int y, uint radius, uint attribute, vector<Map_Object*> *list)
 *
 * Find Map_Objects in the given area that have the requested attribute.
 * If radius is 0, only the field x/y is checked.
 *
 * If list is non-zero, pointers to the relevant objects will be stored in the list.
 *
 * Returns true if objects could be found
 */
bool Map::find_objects(int x, int y, uint radius, uint attribute, std::vector<Map_Object*> *list)
{
	Map_Region mr(x, y, radius, this);
	Field *f;
	bool found = false;
	
	while((f = mr.next())) {
		Map_Object *obj = f->get_first_object();
		
		while(obj) {
			if (obj->has_attribute(attribute)) {
				if (list) {
					list->push_back(obj);
					found = true;
				} else
					return true; // no need to look any further
			}
		
			obj = obj->get_next_object();
		}
	}
	
	return found;
}


/*
Field attribute recalculation passes
------------------------------------

Some events can change the map in a way that run-time calculated attributes
(Field::brightness and Field::caps) need to be recalculated.

These events include:
- change of height (e.g. by planing)
- insertion of a "robust" Map_Object
- removal of a "robust" Map_Object

All these events can change the passability, buildability, etc. of fields
with a radius of two fields. This means that you must build a list of the
directly affected field and all fields that can be reached in two steps.

You must then perform the following operations:
1. Call recalc_brightness() and recalc_fieldcaps_pass1() on all fields
2. Call recalc_fieldcaps_pass2() on all fields

Note: it is possible to leave out recalc_brightness() unless the height has
been changed.

The Field::caps calculation is split into more passes because of inter-field
dependencies.
*/

/** Map::recalc_brightness(int fx, int fy, Field *f)
 *
 * Fetch the slopes to neighbours and call the actual logic in Field
 */
void Map::recalc_brightness(int fx, int fy, Field *f)
{
   Field *n;
   int dx, dy;
   int l, r, tl, tr, bl, br;

   f = get_field(fx, fy);

   // left
   get_ln(fx, fy, f, &dx, &dy, &n);
   l = f->get_height() - n->get_height();

   // right
   get_rn(fx, fy, f, &dx, &dy, &n);
   r = f->get_height() - n->get_height();

   // top-left
   get_tln(fx, fy, f, &dx, &dy, &n);
   tl = f->get_height() - n->get_height();

   // top-right
   get_rn(dx, dy, n, &dx, &dy, &n);
   tr = f->get_height() - n->get_height();

   // bottom-left
   get_bln(fx, fy, f, &dx, &dy, &n);
   bl = f->get_height() - n->get_height();

   // bottom-right
   get_rn(dx, dy, n, &dx, &dy, &n);
   br = f->get_height() - n->get_height();

   f->set_brightness(l, r, tl, tr, bl, br);
}

/** Map::recalc_fieldcaps_pass1(int fx, int fy, Field *f)
 *
 * Recalculate the fieldcaps for the given field.
 *  - Check terrain types for passability and flag buildability
 *
 * I hope this is understandable and maintainable. 
 *
 * Note: due to inter-field dependencies, fieldcaps calculations are split up
 * into two passes. You should always perform both passes. See the comment
 * above recalc_brightness.
 */
void Map::recalc_fieldcaps_pass1(int fx, int fy, Field *f)
{
	f->caps = 0;

	// 1a) Get all the neighbours to make life easier
	int tlnx, tlny, trnx, trny, lnx, lny, rnx, rny, blnx, blny, brnx, brny;
	Field *tln, *trn, *ln, *rn, *bln, *brn;
	
	get_tln(fx, fy, f, &tlnx, &tlny, &tln);
	get_trn(fx, fy, f, &trnx, &trny, &trn);
	get_ln(fx, fy, f, &lnx, &lny, &ln);
	get_rn(fx, fy, f, &rnx, &rny, &rn);
	get_bln(fx, fy, f, &blnx, &blny, &bln);
	get_brn(fx, fy, f, &brnx, &brny, &brn);

	// 1b) Collect some information about the neighbours
	int cnt_unpassable = 0;
	int cnt_water = 0;
	int cnt_acid = 0;
	
	if (f->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (f->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (ln->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (tln->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (tln->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (trn->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	
	if (f->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (f->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
	if (ln->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (tln->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
	if (tln->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (trn->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;

	if (f->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (f->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (ln->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (tln->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (tln->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (trn->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	
		
	// 2) Passability
	
	// 2a) If any of the neigbouring triangles is walkable this field is
	//     walkable.
	if (cnt_unpassable < 6)
		f->caps |= MOVECAPS_WALK;
	
	// 2b) If all neighbouring triangles are water, the field is swimable
	if (cnt_water == 6)
		f->caps |= MOVECAPS_SWIM;
	
	// 2c) [OVERRIDE] If any of the neighbouring triangles is really
	//     "bad" (such as lava), we can neither walk nor swim to this field.
	if (cnt_acid)
		f->caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);
	
	// === everything below is used to check buildability ===
		
	// 3) General buildability check: if a "robust" Map_Object is on this field
	//    we cannot build anything on it
	if (find_objects(fx, fy, 0, Map_Object::ROBUST, 0))
	{
		// 3b) [OVERRIDE] check for "unpassable" Map_Objects
		if (find_objects(fx, fy, 0, Map_Object::UNPASSABLE, 0))
			f->caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);
		return;
	}
	
	// 4) Flags
	//    We can build flags on anything that's walkable and buildable, with some 
	//    restrictions
	if (f->caps & MOVECAPS_WALK)
	{
		// 4b) Flags must be at least 1 field apart
		if (!find_objects(fx, fy, 1, Map_Object::FLAG, 0))
			f->caps |= BUILDCAPS_FLAG;
	}
}

/** Map::recalc_fieldcaps_pass2(int fx, int fy, Field *f)
 *
 * Second pass of fieldcaps. Determine which kind of building (if any) can be built
 * on this Field.
 *
 * Important: flag buildability has already been checked in the first pass.
 */
void Map::recalc_fieldcaps_pass2(int fx, int fy, Field *f)
{
	// 1) Collect neighbour information
	//
	// NOTE: Yes, this reproduces some of the things done in pass1.
	// This is unavoidable and shouldn't hurt too much anyway.

	// 1a) Get all the neighbours to make life easier
	int tlnx, tlny, trnx, trny, lnx, lny, rnx, rny, blnx, blny, brnx, brny;
	Field *tln, *trn, *ln, *rn, *bln, *brn;
	
	get_tln(fx, fy, f, &tlnx, &tlny, &tln);
	get_trn(fx, fy, f, &trnx, &trny, &trn);
	get_ln(fx, fy, f, &lnx, &lny, &ln);
	get_rn(fx, fy, f, &rnx, &rny, &rn);
	get_bln(fx, fy, f, &blnx, &blny, &bln);
	get_brn(fx, fy, f, &brnx, &brny, &brn);

	// 1b) Collect some information about the neighbours
	int cnt_unpassable = 0;
	int cnt_water = 0;
	int cnt_acid = 0;
	int cnt_mountain = 0;
	int cnt_dry = 0;
	
	if (f->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (f->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (ln->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (tln->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (tln->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (trn->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	
	if (f->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (f->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
	if (ln->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (tln->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
	if (tln->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (trn->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;

	if (f->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (f->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (ln->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (tln->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (tln->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (trn->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	
	if (f->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (f->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (ln->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (tln->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (tln->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (trn->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	
	if (f->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (f->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (ln->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (tln->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (tln->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (trn->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;
	
	// 2) We can only build something on fields that are
	//     - walkable
	//     - have no water triangles next to them
	//     - are not blocked by "robust" Map_Objects
	if (!(f->caps & MOVECAPS_WALK) ||
	    cnt_water ||
	    find_objects(fx, fy, 0, Map_Object::ROBUST, 0))
		return;
		
	// 3) We can only build something if there is a flag on the bottom-right neighbour
	//    (or if we could build a flag on the bottom-right neighbour)
	//
	// NOTE: This dependency on the bottom-right neighbour is the reason why the caps
	// calculation is split into two passes
	if (!(brn->caps & BUILDCAPS_FLAG) &&
	    !find_objects(brnx, brny, 0, Map_Object::FLAG, 0))
		return;
	
	// === passability and flags allow us to build something beyond this point ===

	// 4) Build mines on mountains
	if (cnt_mountain == 6)
	{
		// 4b) Check the mountain slope
		if ((int)brn->get_height() - f->get_height() < 4)
			f->caps |= BUILDCAPS_MINE;
		return;
	}
	
	// 5) Can't build anything if there are mountain or desert triangles next to the field
	if (cnt_mountain || cnt_dry)
		return;
		
	// 6) TODO: Reduce building size if buildings/roads/robusts(?) are near
   // TODO: even worse: in S2 stones (granit) laying around reduce all their neighbours to flags
   // while trees only reduces their neighbours to small buildings (except it's top-left neighbour, obviously -Holger
	uchar building = BUILDCAPS_BIG;
	bool harbour = false;
	
	// 7) Reduce building size based on slope of direct neighbours:
	//    - slope >= 4: can't build anything here -> return
	//    - slope >= 3: maximum size is small
	int slope;

	slope = abs((int)tln->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;
	
	slope = abs((int)trn->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;
	
	slope = abs((int)rn->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;

	// Special case for bottom-right neighbour (where our flag is)
	// Is this correct?	
   // Yep, it is - Holger
	slope = abs((int)brn->get_height() - f->get_height());
	if (slope >= 2) return;
	
	slope = abs((int)bln->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;
	
	slope = abs((int)ln->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;

	// 8) Reduce building size based on height diff. of second order neighbours
	//     If height difference between this field and second order neighbour
	//     is >= 3, we can only build a small house here.
	//    Additionally, we can potentially build a harbour on this field if one
	//    of the second order neighbours is swimmable.
   //    TODO: didn't we agree that it would be easier to make harbours only on specially set
   //    fields? - Holger
	//
	// Processes the neighbours in clockwise order, starting from the right-most
	int secx, secy;
	Field *sec;

	get_rn(rnx, rny, rn, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;
	
	get_brn(rnx, rny, rn, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_brn(brnx, brny, brn, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_bln(brnx, brny, brn, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_bln(blnx, blny, bln, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_ln(blnx, blny, bln, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_ln(lnx, lny, ln, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_tln(lnx, lny, ln, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_tln(tlnx, tlny, tln, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_trn(tlnx, tlny, tln, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_trn(trnx, trny, trn, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_rn(trnx, trny, trn, &secx, &secy, &sec);
	if (sec->caps & MOVECAPS_SWIM) harbour = true;
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	// 9) That's it, store the collected information
	f->caps |= building;
	//if (building == BUILDCAPS_BIG && harbour)
	//	f->caps |= BUILDCAPS_PORT;
}


/** Map::recalc_for_field(int fx, int fy)
 *
 * Call this function whenever the field at fx/fy has changed in one of the ways:
 *  - height has changed
 *  - robust Map_Object has been added or removed
 *
 * This performs the steps outlined in the comment above recalc_brightness()
 */
void Map::recalc_for_field(int fx, int fy)
{
	Map_Region_Coords mrc;
	int x, y;
	Field *f;
	
	// First pass
	mrc.init(fx, fy, 2, this);

	while(mrc.next(&x, &y)) {
		f = get_field(x, y);
		recalc_brightness(x, y, f);
		recalc_fieldcaps_pass1(x, y, f);
	}
	
	// Second pass
	mrc.init(fx, fy, 2, this);
	
	while(mrc.next(&x, &y)) {
		f = get_field(x, y);
		recalc_fieldcaps_pass2(x, y, f);
	}
}
	
/** Field::Build_Symbol Map::get_build_symbol(int x, int y)
 *
 * This function returns the build symbol on this field.
 *
 * This is done by consulting the appropriate Field::caps
 */
Field::Build_Symbol Map::get_build_symbol(int x, int y)
{
   Field *f;
	
	f = get_safe_field(x, y);

	if (f->caps & BUILDCAPS_PORT)
		return Field::PORT;
	if (f->caps & BUILDCAPS_MINE)
		return Field::MINE;
		
	switch(f->caps & BUILDCAPS_SIZEMASK) {
	case BUILDCAPS_BIG: return Field::BIG;
	case BUILDCAPS_MEDIUM: return Field::MEDIUM;
	case BUILDCAPS_SMALL: return Field::SMALL;
	}
	
	if (f->caps & BUILDCAPS_FLAG)
		return Field::FLAG;
		
	return Field::NOTHING;
}

/** Map::calc_distance(Coords a, Coords b)
 *
 * Calculate the (Manhattan) distance from a to b
 * a and b are expected to be normalized!
 */
int Map::calc_distance(Coords a, Coords b)
{
	uint dist;
	int dy;
	
	// do we fly up or down?
	dy = b.y - a.y;
	if (dy > (int)(hd.height>>1)) // wrap-around!
		dy -= hd.height;
	else if (dy < -(int)(hd.height>>1))
		dy += hd.height;
	
	dist = abs(dy);
	
	if (dist >= hd.width) // no need to worry about x movement at all
		return dist;
	
	// [lx..rx] is the x-range we can cover simply by walking vertically
	// towards b
	// Hint: (~a.y & 1) is 1 for even rows, 0 for odd rows.
	// This means we round UP for even rows, and we round DOWN for odd rows.
	int lx, rx;
	
	lx = a.x - ((dist + (~a.y & 1))>>1); // div 2
	rx = lx + dist;
	
	// Allow for wrap-around
	// Yes, the second is an else if; see the above if (dist >= hd.width)
	if (lx < 0)
		lx += hd.width;
	else if (rx >= (int)hd.width)
		rx -= hd.width;
	
	// Normal, non-wrapping case
	if (lx <= rx)
	{
		if (b.x < lx)
		{
			int dx1 = lx - b.x;
			int dx2 = b.x - (rx - hd.width);
			dist += min(dx1, dx2);
		}
		else if (b.x > rx)
		{
			int dx1 = b.x - rx;
			int dx2 = (lx + hd.width) - b.x;
			dist += min(dx1, dx2);
		}
	}
	else
	{
		// Reverse case
		if (b.x > rx && b.x < lx)
		{
			int dx1 = b.x - rx;
			int dx2 = lx - b.x;
			dist += min(dx1, dx2);
		}
	}
	
//	cerr << a.x << "," << a.y << " -> " << b.x << "," << b.y << " : " << dist << endl;
	
	return dist;
}

/** class StarQueue
 *
 * Provides the flexible priority queue to maintain the open list.
 */
class StarQueue {
	std::vector<Map::Pathfield*> m_data;

public:
	StarQueue() { }
	~StarQueue() { }

	void flush() { m_data.clear(); }

	// Return the best node and readjust the tree
	// Basic idea behind the algorithm:
	//  1. the top slot of the tree is empty
	//  2. if this slot has both children:
	//       fill this slot with one of its children or with slot[_size], whichever
	//       is best;
	//       if we filled with slot[_size], stop
	//       otherwise, repeat the algorithm with the child slot
	//     if it doesn't have any children (l >= _size)
	//       put slot[_size] in its place and stop
	//     if only the left child is there
	//       arrange left child and slot[_size] correctly and stop
	Map::Pathfield* pop()
	{
		if (m_data.empty())
			return 0;

		Map::Pathfield* head = m_data[0];

		unsigned nsize = m_data.size()-1;
		unsigned fix = 0;
		while(fix < nsize) {
			unsigned l = fix*2 + 1;
			unsigned r = fix*2 + 2;
			if (l >= nsize) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->heap_index = fix;
				break;
			}
			if (r >= nsize) {
				if (m_data[nsize]->cost() <= m_data[l]->cost()) {
					m_data[fix] = m_data[nsize];
					m_data[fix]->heap_index = fix;
				} else {
					m_data[fix] = m_data[l];
					m_data[fix]->heap_index = fix;
					m_data[l] = m_data[nsize];
					m_data[l]->heap_index = l;
				}
				break;
			}

			if (m_data[nsize]->cost() <= m_data[l]->cost() && m_data[nsize]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->heap_index = fix;
				break;
			}
			if (m_data[l]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[l];
				m_data[fix]->heap_index = fix;
				fix = l;
			} else {
				m_data[fix] = m_data[r];
				m_data[fix]->heap_index = fix;
				fix = r;
			}
		}
		
		m_data.pop_back();

		debug(0, "pop");
		
		head->heap_index = -1;
		return head;
	}

	// Add a new node and readjust the tree
	// Basic idea:
	//  1. Put the new node in the last slot
	//  2. If parent slot is worse than self, exchange places and recurse
	// Note that I rearranged this a bit so swap isn't necessary
	void push(Map::Pathfield *t)
	{
		unsigned slot = m_data.size();
		m_data.push_back(0);

		while(slot > 0) {
			unsigned parent = (slot - 1) / 2;

			if (m_data[parent]->cost() < t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->heap_index = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->heap_index = slot;
		
		debug(0, "push");
	}

	// Rearrange the tree after a node has become better, i.e. move the
	// node up
	// Pushing algorithm is basically the same as in push()
	void boost(Map::Pathfield *t)
	{
		unsigned slot = t->heap_index;

		assert(m_data[slot] == t);

		while(slot > 0) {
			unsigned parent = (slot - 1) / 2;

			if (m_data[parent]->cost() <= t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->heap_index = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->heap_index = slot;
		
		debug(0, "boost");
	}

	// Recursively check integrity
	void debug(unsigned node, const char *str)
	{
		unsigned l = node*2 + 1;
		unsigned r = node*2 + 2;
		if (m_data[node]->heap_index != (int)node) {
			fprintf(stderr, "%s: heap_index integrity!\n", str);
			exit(-1);
		}
		if (l < m_data.size()) {
			if (m_data[node]->cost() > m_data[l]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				exit(-1);
			}
			debug(l, str);
		}
		if (r < m_data.size()) {
			if (m_data[node]->cost() > m_data[r]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				exit(-1);
			}
			debug(r, str);
		}
	}

};

/** Map::findpath(Coords start, Coords end, uchar movecaps, int persist, Path *path)
 *
 * Pathfinding entry-point.
 * Finds a path from startx/starty to endx/endy for a Map_object with the given movecaps.
 *
 * The path is stored in path, in terms of Map_Object::MOVE_* constants.
 * persist tells the function how hard it should try to find a path:
 * If persist is 0, the function will never give up early. Otherwise, the
 * function gives up when it becomes clear that the path must be longer than
 * persist*bird's distance fields long. 
 * [not implement right now]: If the terrain contains lots of hills, the cost
 * calculation will cause the search to terminate earlier than this. If persist==1,
 * findpath() can only find a path if the terrain is completely flat.
 *
 * The function returns the cost of the path (in milliseconds of normal walking
 * speed) or -1 if no path has been found.
 *
 * TODO: terrain impacts movement speed
 */
#define COST_PER_FIELD		2000 // TODO
 
int Map::findpath(Coords start, Coords end, uchar movecaps, int persist, Path *path)
{
	Field *startf, *endf;
	int upper_cost_limit;
	Field *curf;
	Coords cur;

	path->m_path.clear();
	
	normalize_coords(&start.x, &start.y);
	normalize_coords(&end.x, &end.y);
	
	// Some stupid cases...
	if (start == end) {
		path->m_map = this;
		path->m_start = start;
		path->m_end = end;
		return 0; // duh...
	}
		
	startf = get_field(start);
	if (!(startf->get_caps() & movecaps)) {
		if (!(movecaps & MOVECAPS_SWIM))
			return -1;
		
		if (!can_reach_by_water(start))
			return -1;
	}

	endf = get_field(end);
	if (!(endf->get_caps() & movecaps)) {
		if (!(movecaps & MOVECAPS_SWIM))
			return -1;
		
		if (!can_reach_by_water(end))
			return -1;
	}
	
	// Increase the counter
	// If the counter wrapped, clear the entire pathfinding array
	// This means we clear the array only every 65536 runs
	m_pathcycle++;
	if (!m_pathcycle) {
		memset(m_pathfields, 0, sizeof(Pathfield)*hd.height*hd.width);
		m_pathcycle++;
	}
	
	if (!persist)
		upper_cost_limit = 0;
	else
		upper_cost_limit = persist * calc_distance(start, end) * COST_PER_FIELD; // assume 2 secs per field
	
	// Actual pathfinding
	StarQueue Open;
	Pathfield *curpf;
	
	curpf = m_pathfields + (startf-fields);
	curpf->cycle = m_pathcycle;
	curpf->real_cost = 0;
	curpf->estim_cost = calc_distance(start, end) * COST_PER_FIELD;
	curpf->backlink = Map_Object::IDLE;
	
	Open.push(curpf);

	while((curpf = Open.pop()))
	{
		curf = fields + (curpf-m_pathfields);
		
		get_coords(curf, &cur);

		if (upper_cost_limit && curpf->real_cost > upper_cost_limit)
			break; // upper cost limit reached, give up
		if (curf == endf)
			break; // found our target

		// avoid bias by using different orders when pathfinding
		static const char order1[6] = { Map_Object::WALK_NW, Map_Object::WALK_NE, 
			Map_Object::WALK_E, Map_Object::WALK_SE, Map_Object::WALK_SW, Map_Object::WALK_W };
		static const char order2[6] = { Map_Object::WALK_NW, Map_Object::WALK_W, 
			Map_Object::WALK_SW, Map_Object::WALK_SE, Map_Object::WALK_E, Map_Object::WALK_NE };
		const char *direction;
		
		if ((cur.x+cur.y) & 1)
			direction = order1;
		else
			direction = order2;

		// Check all the 6 neighbours		
		for(uint i = 6; i; i--, direction++) {
			Field *neighbf;
			Pathfield *neighbpf;
			Coords neighb;
			int cost;
		
			switch(*direction) {
			case Map_Object::WALK_NW: get_tln(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_NE: get_trn(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_E: get_rn(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_SE: get_brn(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_SW: get_bln(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_W: get_ln(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			}
			
			neighbpf = m_pathfields + (neighbf-fields);
			
			// Is the field Closed already?
			if (neighbpf->cycle == m_pathcycle && neighbpf->heap_index < 0)
				continue;
			
			// Calculate cost and passability
			if (!(neighbf->get_caps() & movecaps)) {
				if (!(curf->get_caps() & movecaps & MOVECAPS_SWIM))
					continue;
			}
			
			cost = curpf->real_cost + COST_PER_FIELD;

			if (neighbpf->cycle != m_pathcycle) {
				// add to open list
				neighbpf->cycle = m_pathcycle;
				neighbpf->real_cost = cost;
				neighbpf->estim_cost = calc_distance(neighb, end) * COST_PER_FIELD;
				neighbpf->backlink = *direction;
				Open.push(neighbpf);
			} else if (neighbpf->cost() > cost+neighbpf->estim_cost) {
				// found a better path to a field that's already Open
				neighbpf->real_cost = cost;
				neighbpf->backlink = *direction;
				Open.boost(neighbpf);
			}
		}
	}
	if (!curpf) // there simply is no path
		return -1;
	
	// Now unwind the taken route (even if we couldn't find a complete one!)
	int retval;
	
	if (curf == endf)
		retval = curpf->real_cost;
	else
		retval = -1;
	
	path->m_map = this;
	path->m_start = start;
	path->m_end = cur;
	
	path->m_path.clear();
	
	while(curpf->backlink != Map_Object::IDLE) {
		path->m_path.push_back(curpf->backlink);
		
		// Reverse logic! (WALK_NW needs to find the SE neighbour)
		switch(curpf->backlink) {
		case Map_Object::WALK_NW: get_brn(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_NE: get_bln(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_E: get_ln(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_SE: get_tln(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_SW: get_trn(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_W: get_rn(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		}

		curpf = m_pathfields + (curf-fields);
	}
	
	return retval;
}

/** Map::can_reach_by_water(Coords field)
 *
 * We can reach a field by water either if it has MOVECAPS_SWIM or if it has
 * MOVECAPS_WALK and at least one of the neighbours has MOVECAPS_SWIM
 */
bool Map::can_reach_by_water(Coords field)
{
	Field *f = get_field(field);
	
	if (f->get_caps() & MOVECAPS_SWIM)
		return true;
	if (!(f->get_caps() & MOVECAPS_WALK))
		return false;
	
	Coords n;
	Field *nf;

	get_tln(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_trn(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_rn(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_brn(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_bln(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_ln(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;
	
	return false;
}

// 
// class Map_Region
// 
void Map_Region::init(int x, int y, int area, Map *m)
{
	backwards=0;
	_area=area;
	_map=m;
	_lf=m->get_safe_field(x, y);
	_tl=_tr=_bl=_br=_lf;
	tlx=trx=blx=brx=x;
	tly=tr_y=bly=bry=y;
	sx=x; sy=y;
	int i;
	for(i=0; i<area; i++) {
		m->get_tln(tlx, tly, _tl, &tlx, &tly, &_tl);
		m->get_trn(trx, tr_y, _tr, &trx, &tr_y, &_tr);
		m->get_bln(blx, bly, _bl, &blx, &bly, &_bl);
		m->get_brn(brx, bry, _br, &brx, &bry, &_br);
	}
	_lf=_tl;
	cx=tlx; cy=tly;
}
     

Field* Map_Region::next(void) {
   Field* retval=_lf;

   if(_lf==_tr) {
      if(tly==sy) backwards=true;
      if(backwards) {
          if(_lf==_br) {
             _lf=0;
             return retval;
          }
         _map->get_brn(tlx, tly, _tl, &tlx, &tly, &_tl);
         _map->get_bln(trx, tr_y, _tr, &trx, &tr_y, &_tr);
         _lf=_tl;
         cx=tlx;
         cy=tly;
      } else {
         _map->get_bln(tlx, tly, _tl, &tlx, &tly, &_tl);
         _map->get_brn(trx, tr_y, _tr, &trx, &tr_y, &_tr);
         _lf=_tl;
         cx=tlx;
         cy=tly;
         return retval;
      }
   } else {
      _map->get_rn(cx, cy, _lf, &cx, &cy, &_lf);
   }

   
   return retval;
}

// 
// class Map_Region_Coords
// 
void Map_Region_Coords::init(int x, int y, int area, Map *m)
{
	backwards=0;
	_area=area;
	_map=m;
	_lf=m->get_safe_field(x, y);
	_tl=_tr=_bl=_br=_lf;
	tlx=trx=blx=brx=x;
	tly=tr_y=bly=bry=y;
	sx=x; sy=y;
	int i;
	for(i=0; i<area; i++) {
		m->get_tln(tlx, tly, _tl, &tlx, &tly, &_tl);
		m->get_trn(trx, tr_y, _tr, &trx, &tr_y, &_tr);
		m->get_bln(blx, bly, _bl, &blx, &bly, &_bl);
		m->get_brn(brx, bry, _br, &brx, &bry, &_br);
	}
	_lf=_tl;
	cx=tlx; cy=tly;
}

int Map_Region_Coords::next(int* retx, int* rety) {
   int retval=0;
 
   if(_lf) retval=1;
   *retx=cx;
   *rety=cy;
      

   if(_lf==_tr) {
      if(tly==sy) backwards=true;
      if(backwards) {
          if(_lf==_br) {
             _lf=0;
             return retval;
          }
         _map->get_brn(tlx, tly, _tl, &tlx, &tly, &_tl);
         _map->get_bln(trx, tr_y, _tr, &trx, &tr_y, &_tr);
         _lf=_tl;
         cx=tlx;
         cy=tly;
      } else {
         _map->get_bln(tlx, tly, _tl, &tlx, &tly, &_tl);
         _map->get_brn(trx, tr_y, _tr, &trx, &tr_y, &_tr);
         _lf=_tl;
         cx=tlx;
         cy=tly;
         return retval;
      }
   } else {
      _map->get_rn(cx, cy, _lf, &cx, &cy, &_lf);
   }

   
   return retval;
}
