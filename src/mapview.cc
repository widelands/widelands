/*
 * Copyright (C) 2002 by The Widelands Development Team
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
#include "graphic.h"
#include "map.h"
#include "field.h"
#include "mapview.h"
#include "game.h"
#include "player.h"
#include "IntPlayer.h"


AutoPic Map_View::pic_fsel("fsel.bmp", 0, 0, 255);
AutoPic Map_View::small_building("small.bmp", 0, 0, 255);
AutoPic Map_View::medium_building("medium.bmp", 0, 0, 255);
AutoPic Map_View::big_building("big.bmp", 0, 0, 255);
AutoPic Map_View::mine_building("mine.bmp", 0, 0, 255);
AutoPic Map_View::setable_flag("set_flag.bmp", 0, 0, 255);
AutoPic Map_View::pic_roadb_green("roadb_green.bmp", 0, 0, 255);
AutoPic Map_View::pic_roadb_yellow("roadb_yellow.bmp", 0, 0, 255);
AutoPic Map_View::pic_roadb_red("roadb_red.bmp", 0, 0, 255);

/*
===============
Map_View::Map_View

Initialize
===============
*/
Map_View::Map_View(Panel *parent, int x, int y, uint w, uint h, Interactive_Player *player)
	: Panel(parent, x, y, w, h)
{
	m_player = player;
	m_game = player->get_game();
	m_map = m_game->get_map();
	
	vpx = vpy = 0;
	dragging = false;

	show_buildhelp = false;
}

/*
===============
Map_View::~Map_View

Cleanups
===============
*/
Map_View::~Map_View(void)
{
}


/*
===============
Map_View::warp_mouse_to_field

Moves the mouse cursor so that it is directly above the given field
===============
*/
void Map_View::warp_mouse_to_field(Coords c)
{
	int x, y;

	m_map->get_pix(c, &x, &y);
	x -= vpx;
	y -= vpy;
	
	if (x >= 0 && x < get_w() && y >= 0 && y < get_h())
		set_mouse_pos(x, y);
}


/*
===============
Map_View::draw
 
This is the guts!! this function draws the whole
map the user can see. we spend a lot of time
in this function
===============
*/
void Map_View::draw(Bitmap *bmp, int ofsx, int ofsy)
{
	// Prepare an improved bitmap which we can draw into without using ofsx/ofsy
	int effvpx = vpx;
	int effvpy = vpy;
	Bitmap dst;
	bool use_see_area = !m_player->get_ignore_shadow();
	
	if (!dst.make_partof(bmp, ofsx, ofsy, get_w(), get_h(), &ofsx, &ofsy))
		return;
	
	if (ofsx < 0)
		effvpx -= ofsx;
	if (ofsy < 0)
		effvpy -= ofsy;

	if (use_see_area) {
   	dst.fill_rect(0, 0, dst.get_w(), dst.get_h(), pack_rgb(0,0,0));
	}

	draw_ground(&dst, effvpx, effvpy, use_see_area);

	// debug: show fsel coordinates
	Coords fsel = m_player->get_fieldsel();
	char buf[16];
	sprintf(buf, "%i %i", fsel.x, fsel.y);
	Pic *p = g_fh.get_string(buf, 0);
	copy_pic(bmp, p, ofsx+5, ofsy+5, 0, 0, p->get_w(), p->get_h());
	delete p;
}

/*
===============
Map_View::draw_ground

Draw the Map. ground, bobs, you_name_it
effvpx/effvpy are the viewpoint coordinates
===============
*/
void Map_View::draw_ground(Bitmap *dst, int effvpx, int effvpy, bool use_see_area)
{
   // TODO: change this function name. it's not really about drawing the ground only
	Player *player = m_player->get_player();
   int minfx, minfy;
   int maxfx, maxfy;

   minfx = (effvpx + (FIELD_WIDTH>>1)) / FIELD_WIDTH - 1; // hack to prevent negative numbers
   minfy = effvpy / (FIELD_HEIGHT>>1);
   maxfx = (effvpx + (FIELD_WIDTH>>1) + dst->get_w()) / FIELD_WIDTH;
   maxfy = (effvpy + dst->get_h()) / (FIELD_HEIGHT>>1);
   maxfy += 10; // necessary because of heights

   //	printf("%i %i -> %i %i\n", minfx, minfy, maxfx, maxfy);
   int dx = maxfx - minfx + 1;
   int dy = maxfy - minfy + 1;
   int linear_fy = minfy;

   while(dy--) {
      int linear_fx = minfx;
      int fx, fy;
      int bl_x, bl_y;
      int tl_x, tl_y;
      Field *f, *f_bl, *f_tl;
      int posx, posy;
      int blposx, bposy;
      int tlposx, tposy;

      // Use linear (non-wrapped) coordinates to calculate on-screen pos
      m_map->get_basepix(linear_fx, linear_fy, &posx, &posy);
      posx -= effvpx;
      posy -= effvpy;

      // Get linear bottom-left coordinate
      bl_y = linear_fy+1;
      bl_x = linear_fx - (bl_y&1);

      m_map->get_basepix(bl_x, bl_y, &blposx, &bposy);
      blposx -= effvpx;
      bposy -= effvpy;

      // Get linear top-left coordinates 
      tl_y = linear_fy-1;
      tl_x = linear_fx - (tl_y&1);

      m_map->get_basepix(tl_x, tl_y, &tlposx, &tposy);
      tlposx -= effvpx;
      tposy -= effvpy;

      // Calculate safe (bounded) field coordinates and get field pointers
      fx = linear_fx;
      fy = linear_fy;
      m_map->normalize_coords(&fx, &fy);
      m_map->normalize_coords(&bl_x, &bl_y);
      m_map->normalize_coords(&tl_x, &tl_y);
      f = m_map->get_field(fx, fy);
      f_bl = m_map->get_field(bl_x, bl_y);
      f_tl = m_map->get_field(tl_x, tl_y);

      int count = dx;
      while(count--) {
         Field *f_br, *f_r, *f_l, *f_tr;
         int rposx, brposx, lposx, trposx;
         int r_x, r_y, br_x, br_y, l_x, l_y, tr_x, tr_y; 
         bool render_r=true;
         bool render_b=true;

         m_map->get_rn(fx, fy, f, &r_x, &r_y, &f_r);
         rposx = posx + FIELD_WIDTH;

         m_map->get_ln(fx, fy, f, &l_x, &l_y, &f_l);
         lposx = posx - FIELD_WIDTH;

         m_map->get_rn(bl_x, bl_y, f_bl, &br_x, &br_y, &f_br);
         brposx = blposx + FIELD_WIDTH;

         m_map->get_rn(tl_x, tl_y, f_tl, &tr_x, &tr_y, &f_tr);
         trposx = tlposx + FIELD_WIDTH;

			if (use_see_area) {
				if (!player->is_field_seen(fx, fy) ||
				    !player->is_field_seen(br_x, br_y)) {
					render_r=false;
					render_b=false;
				} else {
					if(!player->is_field_seen(bl_x, bl_y))
						render_b=false;
					if(!player->is_field_seen(r_x, r_y))
						render_r=false;
				}
			}
			
			// Render stuff that belongs to ground triangles
			if (render_b || render_r) {
				uchar roads = f->get_roads();
				
				if (m_player->is_building_road())
					roads |= get_overlay_roads(FCoords(fx, fy, f));

         	draw_field(dst, f, f_r, f_bl, f_br, posx, rposx, posy, blposx, brposx, bposy, roads, render_r, render_b);
			}	
			
			// Render stuff that belongs to the field node
			if (!use_see_area || player->is_field_seen(fx, fy))
			{
				// Render frontier
				// Note: Frontiers hot spot must be the lowest y-pixel in the picture
				// otherwise, the bob will get 'clipped' (read: overdrawn) by the fields that are drawn
				// below this field.
				// This could be avoided by either pre-rendering all ground-fields before drawing a bob (time-costy!)
				// or by different (more complicated, also with build-help) checking here below.  - Holger
				if(f->get_owned_by()) {
					Player *ownerplayer = m_game->get_player(f->get_owned_by());
					Animation *frontier = ownerplayer->get_tribe()->get_frontier_anim();
					const uchar *playercolors = ownerplayer->get_playercolor_rgb();
				
					if(f_tl->get_owned_by() != f->get_owned_by()) {
						copy_animation_pic(dst, frontier, 0, 
								tlposx, tposy - f_tl->get_height()*HEIGHT_FACTOR, playercolors);
						// left to top-left
						if(f_l->get_owned_by() != f->get_owned_by()) {
							copy_animation_pic(dst,  frontier, 0, 
									(lposx+tlposx)>>1, ((posy - f_l->get_height()*HEIGHT_FACTOR)+(tposy - f_tl->get_height()*HEIGHT_FACTOR))>>1,
									playercolors);
						}
						// top-left to top-right
						if(f_tr->get_owned_by() != f->get_owned_by()) {
							copy_animation_pic(dst, frontier, 0,
									(tlposx+trposx)>>1, ((tposy - f_tl->get_height()*HEIGHT_FACTOR)+(tposy - f_tr->get_height()*HEIGHT_FACTOR))>>1,
									playercolors);
						}
					}
					if(f_br->get_owned_by() != f->get_owned_by()) {
						copy_animation_pic(dst,  frontier, 0, 
								brposx, bposy - f_br->get_height()*HEIGHT_FACTOR, playercolors);
						// bottom-right to right
						if(f_r->get_owned_by() != f->get_owned_by()) {
							copy_animation_pic(dst, frontier, 0,
									(brposx+rposx)>>1, ((bposy - f_br->get_height()*HEIGHT_FACTOR)+(posy - f_r->get_height()*HEIGHT_FACTOR))>>1,
									playercolors);
						}
						// bottom-left to bottom-left
						if(f_bl->get_owned_by() != f->get_owned_by()) {
							copy_animation_pic(dst, frontier, 0,
									(brposx+blposx)>>1, ((bposy - f_br->get_height()*HEIGHT_FACTOR)+(bposy - f_bl->get_height()*HEIGHT_FACTOR))>>1,
									playercolors);
						}
					}
					// right to top-right
					if(f_tr->get_owned_by() != f->get_owned_by() &&
							f_r->get_owned_by() != f->get_owned_by()) {
						copy_animation_pic(dst,  frontier, 0, 
								rposx, posy - f_r->get_height()*HEIGHT_FACTOR, playercolors);
						copy_animation_pic(dst,  frontier, 0, 
								trposx, tposy - f_tr->get_height()*HEIGHT_FACTOR, playercolors);
						copy_animation_pic(dst, frontier, 0,
								(trposx+rposx)>>1, ((tposy - f_tr->get_height()*HEIGHT_FACTOR)+(posy - f_r->get_height()*HEIGHT_FACTOR))>>1,
								playercolors);
					}
					// left to bottom-left
					if(f_l->get_owned_by() != f->get_owned_by() &&
							f_bl->get_owned_by() != f->get_owned_by()) {
						copy_animation_pic(dst,  frontier, 0, 
								lposx, posy - f_l->get_height()*HEIGHT_FACTOR, playercolors);
						copy_animation_pic(dst,  frontier, 0, 
								blposx, bposy - f_bl->get_height()*HEIGHT_FACTOR, playercolors);
						copy_animation_pic(dst, frontier, 0,
								(blposx+lposx)>>1, ((bposy - f_bl->get_height()*HEIGHT_FACTOR)+(posy - f_l->get_height()*HEIGHT_FACTOR))>>1,
								playercolors);
					}
				}

				// Render bobs
				// TODO - rendering order?
				// This must be defined somewho. some bobs have a higher priority than others
				//  ^-- maybe this priority is a moving vs. non-moving bobs thing?
				// draw_ground implies that this doesn't render map objects.
				// are there any overdraw issues with the current rendering order?

				// Draw Map_Objects hooked to this field
				Map_Object* obj = f->get_first_object();
				while(obj) {
					obj->draw(m_game, dst, posx, posy);
					obj = obj->get_next_object();
				}

				// Draw build-help etc...
				draw_overlay(dst, FCoords(fx, fy, f), posx, posy);
			}
			
			// Advance to next field in row
         f_bl = f_br;
         blposx = brposx;
         bl_x = br_x;
         bl_y = br_y;

         f = f_r;
         posx = rposx;
         fx = r_x;
         fy = r_y;

         f_tl = f_tr;
         tlposx = trposx;
         tl_x = tr_x;
         tl_y = tr_y;
      }

      linear_fy++;
   }
}

/*
===============
Map_View::draw_field

Draw the two triangles and the roads associated with one field.
===============
*/
void Map_View::draw_field(Bitmap *dst, Field * const f, Field * const rf, Field * const fl, Field * const rfl,
           const int posx, const int rposx, const int posy, const int blposx, const int rblposx, const int blposy, 
			  uchar roads, bool render_r, bool render_b)
{
	// points are ordered: right, left, bottom-right, bottom-left
	// note that as long as render_triangle messes with the arrays, we need to
	// copy them to a safe place first
	Point p[4];
	int b[4];

	p[0] = Point(rposx, posy - rf->get_height()*HEIGHT_FACTOR);
	p[1] = Point(posx, posy - f->get_height()*HEIGHT_FACTOR);
	p[2] = Point(rblposx, blposy - rfl->get_height()*HEIGHT_FACTOR);
	p[3] = Point(blposx, blposy - fl->get_height()*HEIGHT_FACTOR);

	if ((p[2].y < 0 && p[3].y < 0) ||
	    (p[0].y >= (int)dst->get_h() && p[1].y >= (int)dst->get_h()))
		return;

	b[0] = rf->get_brightness();
	b[1] = f->get_brightness();
	b[2] = rfl->get_brightness();
	b[3] = fl->get_brightness();

/*
	b[0] += 20; // debug override for shading (make field borders visible)
	b[3] -= 20;
*/

	// Render right triangle
	Point ptmp[3];
	int btmp[3];

   if(render_r) {
		memcpy(ptmp, p, sizeof(Point)*3);
		memcpy(btmp, b, sizeof(int)*3);
      render_triangle(dst, ptmp, btmp, f->get_terr()->get_texture());
	}

	// Render bottom triangle
   if(render_b) {
		memcpy(ptmp, p+1, sizeof(Point)*3);
		memcpy(btmp, b+1, sizeof(int)*3);
      render_triangle(dst, ptmp, btmp, f->get_terd()->get_texture());
	}
	
	// Render roads
	ushort color;
	uchar road;
	
	road = (roads >> Road_East) & Road_Mask;
	if (render_r && road) {
		if (road == Road_Normal)
			color = pack_rgb(192, 192, 192);
		else if (road == Road_Busy)
			color = pack_rgb(96, 96, 96);
		else
			color = pack_rgb(0, 0, 128);
		render_road_horiz(dst, p[1], p[0], color);
	}
	
	road = (roads >> Road_SouthEast) & Road_Mask;
	if (road) {
		if (road == Road_Normal)
			color = pack_rgb(192, 192, 192);
		else if (road == Road_Busy)
			color = pack_rgb(96, 96, 96);
		else
			color = pack_rgb(0, 0, 128);
		render_road_vert(dst, p[1], p[2], color);
	}
	
	road = (roads >> Road_SouthWest) & Road_Mask;
	if (road) {
		if (road == Road_Normal)
			color = pack_rgb(192, 192, 192);
		else if (road == Road_Busy)
			color = pack_rgb(96, 96, 96);
		else
			color = pack_rgb(0, 0, 128);
		render_road_vert(dst, p[1], p[3], color);
	}
}

/*
===============
Map_View::draw_overlay

Draw overlay pictures such as buildhelp.
Note: this is only called for visible fields.
===============
*/
void Map_View::draw_overlay(Bitmap *dst, FCoords coords, int posx, int posy)
{
	posy -= coords.field->get_height() * HEIGHT_FACTOR;

	// Render buildhelp. 
	// Note: this could also go before rendering bobs, since this shouldn't interfere with non moving bobs
	// but so, animals are below the build help, which might reduce weirdness
	// In other words, it can't go before rendering bobs.
	if (show_buildhelp && coords.field->get_owned_by() == m_player->get_player_number()) {
		int buildcaps = m_player->get_player()->get_buildcaps(coords);

		if (buildcaps & BUILDCAPS_PORT)
		{
			// blah
		}
		else if (buildcaps & BUILDCAPS_MINE)
		{
			copy_pic(dst, &mine_building, posx-(mine_building.get_w()>>1),  posy-(mine_building.get_h()>>1), 
					0, 0, mine_building.get_w(), mine_building.get_h());
		}
		else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG)
		{
			copy_pic(dst, &big_building, posx-(big_building.get_w()>>1), posy-(big_building.get_h()>>1),
					0, 0, big_building.get_w(), big_building.get_h());
		}
		else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_MEDIUM)
		{
			copy_pic(dst, &medium_building, posx-(medium_building.get_w()>>1), posy-(medium_building.get_h()>>1), 
					0, 0, medium_building.get_w(), medium_building.get_h());
		}
		else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_SMALL)
		{
			copy_pic(dst, &small_building, posx-(small_building.get_w()>>1), posy-(small_building.get_h()>>1), 
					0, 0, small_building.get_w(), small_building.get_h());
		}
		else if (buildcaps & BUILDCAPS_FLAG)
		{
			copy_pic(dst, &setable_flag, posx-(setable_flag.get_w()>>1), posy-(setable_flag.get_h()), 
					0, 0, setable_flag.get_w(), setable_flag.get_h());
		}
	}

	// Draw road building help
	if (m_player->is_building_road())
		draw_overlay_road(dst, coords, posx, posy);
	
	// Draw the fsel last
	if (m_player->get_fieldsel() == coords) {
		copy_pic(dst, &pic_fsel, posx - (pic_fsel.get_w()/2), posy - (pic_fsel.get_h()/2), 
				0, 0, pic_fsel.get_w(), pic_fsel.get_h());
	}
}


/*
===============
Map_View::draw_overlay_road

Draw the road build help overlay
===============
*/
void Map_View::draw_overlay_road(Bitmap *dst, FCoords coords, int posx, int posy)
{
	Coords tail = m_player->get_build_road_end();
	int dir = m_map->is_neighbour(coords, tail);

	if (dir <= 0)
		return;
	
	int taildir = m_player->get_build_road_end_dir();

	if (dir == taildir) {
		// this is the last but one field of the road
		return;
	}
	
	// figure out whether the road could go here
	int caps = m_player->get_player()->get_buildcaps(coords);
	
	if (!(caps & MOVECAPS_WALK)) // need to be able to walk here
		return;
	
	std::vector<Map_Object*> objs; // can't build on robusts
	if (m_map->find_objects(coords, 0, Map_Object::ROBUST, &objs)) {
		if (!objs[0]->has_attribute(Map_Object::FLAG))
			return;
	}
	
	if (m_player->get_build_road()->get_index(coords) >= 0)
		return; // the road can't cross itself
	
	// Finally, the road can go here.
	Field *ftail = m_map->get_field(tail);
	int slope = abs(coords.field->get_height() - ftail->get_height());
	Pic *pic;
	
	if (slope < 2)
		pic = &pic_roadb_green;
	else if (slope < 4)
		pic = &pic_roadb_yellow;
	else
		pic = &pic_roadb_red;

	copy_pic(dst, pic, posx - (pic->get_w()/2), posy - (pic->get_h()/2), 
			0, 0, pic->get_w(), pic->get_h());
}

/*
===============
Map_View::get_overlay_roads

Figure out which roads need to be rendered during building
===============
*/
uchar Map_View::get_overlay_roads(FCoords coords)
{
	CoordPath *path = m_player->get_build_road();
	uchar roads;
	
	assert(path);
	
	int idx = path->get_index(coords);
	
	if (idx < 0)
		return 0; // no additional roads
	
	roads = 0;
	
	// the road that moves to this field
	if (idx > 0) {
		int dir = get_reverse_dir(path->get_step(idx-1));
		int shift = 2*(dir - Map_Object::WALK_E);
		
		if (shift >= 0 && shift <= 4)
			roads |= Road_Normal << shift;
	}
	
	// the road that leads away from this field
	if (idx < path->get_nsteps()) {
		int dir = path->get_step(idx);
		int shift = 2*(dir - Map_Object::WALK_E);
		
		if (shift >= 0 && shift <= 4)
			roads |= Road_Normal << shift;
	}
	
	return roads;
}


/** Map_View::set_viewpoint(int x, int y)
 *
 * Set the viewpoint to the given screen coordinates
 */
void Map_View::set_viewpoint(int x, int y)
{
	if (vpx == x && vpy == y)
		return;

	vpx=x; vpy=y;
	while(vpx>(int)(FIELD_WIDTH*m_map->get_w()))			vpx-=(FIELD_WIDTH*m_map->get_w());
	while(vpy>(int)((FIELD_HEIGHT*m_map->get_h())>>1))	vpy-=(FIELD_HEIGHT*m_map->get_h())>>1;
	while(vpx< 0)  vpx+=(FIELD_WIDTH*m_map->get_w());
	while(vpy< 0)  vpy+=(FIELD_HEIGHT*m_map->get_h())>>1;

	warpview.call(vpx, vpy);
}

/** Map_View::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Mouseclicks on the map:
 * Right-click: enable/disable dragging
 * Left-click: field action window
 */
bool Map_View::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn == MOUSE_LEFT)
	{
		if (down) {
         track_fsel(x, y);

			fieldclicked.call();
		}
	}
	else if (btn == MOUSE_RIGHT)
	{
		if (down) {
			dragging = true;
			grab_mouse(true);
		} else if (dragging) {
			grab_mouse(false);
			dragging = false;
		}
	}

	return true;
}

/** Map_View::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
 *
 * Scroll the view according to mouse movement.
 */
void Map_View::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
{
	if (!(btns & (1<<MOUSE_RIGHT)))
		dragging = false;

	if (dragging)
	{
		set_rel_viewpoint(xdiff, ydiff);
		set_mouse_pos(x-xdiff, y-ydiff);
	}

	if (!m_player->get_fieldsel_freeze())
		track_fsel(x, y);

	g_gr.needs_fs_update();
}

/*
===============
Map_View::track_fsel(int mx, int my)

Move the fsel to the given mouse position.
Does not honour fieldsel freeze.
===============
*/
void Map_View::track_fsel(int mx, int my)
{
	Coords fsel;
	
	// First of all, get a preliminary field coordinate based on the basic
	// grid (not taking heights into account)
	my += vpy;
	fsel.y = (my + (FIELD_HEIGHT>>2)) / (FIELD_HEIGHT>>1) - 1;

	mx += vpx;
	fsel.x = mx;
	if (fsel.y & 1)
		fsel.x -= FIELD_WIDTH>>1;
	fsel.x = (fsel.x + (FIELD_WIDTH>>1)) / FIELD_WIDTH;

	m_map->normalize_coords(&fsel.x, &fsel.y);

	// Now, fsel point to where we'd be if the field's height was 0.
	// We now recursively move towards the correct field. Because height cannot
	// be negative, we only need to consider the bottom-left or bottom-right neighbour
	int mapheight = m_map->get_h()*(FIELD_HEIGHT>>1);
	int mapwidth = m_map->get_w()*FIELD_WIDTH;
	Field *f;
	int fscrx, fscry;

	f = m_map->get_field(fsel);
	m_map->get_pix(fsel.x, fsel.y, f, &fscrx, &fscry);

	for(;;) {
		Field *bln, *brn;
		int blx, bly, brx, bry;
		int blscrx, blscry, brscrx, brscry;
		bool movebln, movebrn;
		int fd, blnd, brnd;
		int d2;

		m_map->get_bln(fsel.x, fsel.y, f, &blx, &bly, &bln);
		m_map->get_brn(fsel.x, fsel.y, f, &brx, &bry, &brn);

		m_map->get_pix(blx, bly, bln, &blscrx, &blscry);
		m_map->get_pix(brx, bry, brn, &brscrx, &brscry);

		// determine which field the mouse is closer to on the y-axis
		// bit messy because it has to be aware of map wrap-arounds
		fd = my - fscry;
		d2 = my - mapheight - fscry;
		if (abs(d2) < abs(fd))
			fd = d2;

		blnd = blscry - my;
		d2 = blscry - (my - mapheight);
		if (abs(d2) < abs(blnd))
			blnd = d2;
		movebln = blnd < fd;

		brnd = brscry - my;
		d2 = brscry - (my - mapheight);
		if (abs(d2) < abs(brnd))
			brnd = d2;
		movebrn = brnd < fd;

		if (!movebln && !movebrn)
			break;

		// determine which field is closer on the x-axis
		blnd = mx - blscrx;
		d2 = mx - mapwidth - blscrx;
		if (abs(d2) < abs(blnd))
			blnd = d2;

		brnd = brscrx - mx;
		d2 = brscrx - (mx - mapwidth);
		if (abs(d2) < abs(brnd))
			brnd = d2;

		if (brnd < blnd) {
			if (movebrn) {
				f = brn;
				fsel.x = brx;
				fsel.y = bry;
				fscrx = brscrx;
				fscry = brscry;
			} else if (movebln) {
				f = bln;
				fsel.x = blx;
				fsel.y = bly;
				fscrx = blscrx;
				fscry = blscry;
			} else
				break;
		} else {
			if (movebln)  {
				f = bln;
				fsel.x = blx;
				fsel.y = bly;
				fscrx = blscrx;
				fscry = blscry;
			} else if (movebrn) {
				f = brn;
				fsel.x = brx;
				fsel.y = bry;
				fscrx = brscrx;
				fscry = brscry;
			} else
				break;
		}
	}
	
	// Apply the new fieldsel
	m_player->set_fieldsel(fsel);
}
