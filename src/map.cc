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
#include "myfile.h"
#include "worlddata.h"

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
Map::Map(void) {
		  w=0;
		  fields=0;
        starting_pos=0;
}

/** Map::~Map(void)
 *
 * cleanups
 */
Map::~Map(void) {
   if(fields) {
      // WARNING: if Field has to free something, we have to do
      // it here manually!!!
      free(fields);
   }

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
int Map::load_wlmf(const char* file, Game *game) {
   Binary_file f;

   f.open(file, File::READ);
   if(f.get_state() != File::OPEN) {
      return ERR_FAILED;
   }

   // read header:
   f.read(&hd, sizeof(hd));

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
      f.read(&pl, sizeof(pl));
   }

   set_size(hd.width, hd.height);


   // now, read in the fields, one at a time and init the map
   FieldDescr fd;
   uint y;
   Terrain_Descr *td, *tr;
   for(y=0; y<hd.height; y++) {
      for(uint x=0; x<hd.width; x++) {
         f.read(&fd, sizeof(fd));

         // TEMP
         tr=w->get_terrain(fd.tex_r);
         if(!tr) {
            //cerr << "Texture number " << fd.tex_r << " not found in file. Defaults to 0" << endl;
            tr=w->get_terrain(0);
            assert(0); // we should never be here!  
         }
         td=w->get_terrain(fd.tex_d);
         if(!td) {
            // cerr << "Texture number " << fd.tex_d << " not found in file. Defaults to 0" << endl;
            td=w->get_terrain(0);
            assert(0); // we should never be here!  
         }
         // TEMP end

         Field* f=get_field(x,y);
         f->set_height((uchar)fd.height);
         f->set_terrainr(tr);
         f->set_terraind(td);
			f->objects = 0;
      }
   }

   return RET_OK;
}

/** 
 * This functions load a map header, for previewing maps.
 */
int Map::load_map_header(const char* file) {
   int ret=RET_OK;

   if(!strcasecmp(file+(strlen(file)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
   {
      // It ends like a wide lands map file. try to load
      // it as such
      // TODO: do this
      // ret = load_wlmf(file, q);
   }
   else if(!strcasecmp(file+(strlen(file)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX))
   {
      // it is a S2 Map file. load it as such
      ret = load_s2mf_header(file);
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
   int ret;

   if(!strcasecmp(file+(strlen(file)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
   {
      // It ends like a wide lands map file. try to load
      // it as such
      ret = load_wlmf(file, g);
   }
   else if(!strcasecmp(file+(strlen(file)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX))
   {
      // it is a S2 Map file. load it as such
      ret = load_s2mf(file, g);
   }
   else
   {
      assert(0);
      return ERR_FAILED;
   }

   if (ret != RET_OK)
      return ret;

   // Post process the map in the necessary two passes
	Field *f;
   for(uint y=0; y<hd.height; y++) {
      for(uint x=0; x<hd.width; x++) {
			f = get_field(x, y);
         recalc_brightness(x, y, f);
			recalc_fieldcaps_pass1(x, y, f);
		}
	}

   for(uint y=0; y<hd.height; y++) {
      for(uint x=0; x<hd.width; x++) {
			f = get_field(x, y);
			recalc_fieldcaps_pass2(x, y, f);
		}
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
bool Map::find_objects(int x, int y, uint radius, uint attribute, vector<Map_Object*> *list)
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
	Map_Region_Cords mrc;
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
// class Map_Region_Cords
// 
void Map_Region_Cords::init(int x, int y, int area, Map *m)
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

int Map_Region_Cords::next(int* retx, int* rety) {
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
