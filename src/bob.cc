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
#include "world.h"
#include "bob.h"
#include "game.h"
#include "cmd_queue.h"
#include "map.h"
#include "profile.h"

Animation::Animation(void)
{
	npics = 0;
	pics = 0;
	frametime = FRAME_LENGTH;
	w = h = 0;
	hsx = hsy = 0;
}

Animation::~Animation(void)
{ 
	if(npics) {
		uint i; 
		for(i=0; i<npics; i++) {
			free(pics[i].data);
		}
		free(pics);
	}
}

/** Animation::add_pic(ushort size, ushort* data)
 *
 * Adds one frame of raw encoded data to the animation
 */
void Animation::add_pic(ushort size, ushort* data)
{
	if(!pics) {
		pics=(Animation_Pic*) malloc(sizeof(Animation_Pic));
		npics=1;
	} else {
		++npics;
		pics=(Animation_Pic*) realloc(pics, sizeof(Animation_Pic)*npics);
	}
	pics[npics-1].data=(ushort*)malloc(size);
	pics[npics-1].parent=this;
	memcpy(pics[npics-1].data, data, size);
}

/** Animation::add_pic(Pic* pic, bool hasclrkey, ushort clrkey, bool hasshadow, ushort shadowclr)
 *
 * Adds one frame to the animation
 */
void Animation::add_pic(Pic* pic, bool hasclrkey, ushort clrkey, ushort hasshadow, int shadowclr,
                                  int hasplrclrs, ushort *plrclrs)
{
	if(pic->get_w() != w && pic->get_h() != h)
		throw wexception("frame must be %ix%i pixels big", w, h);

   ushort* data = 0;
	uint in, out;
	int runstart = -1; // code field for normal run
	uint npix = pic->get_w()*pic->get_h();
	ushort *pixels = pic->get_pixels();
	
	try
	{
		data = (ushort*) malloc(pic->get_w()*pic->get_h()*sizeof(ushort)*2); 
		
		in = 0;
		out = 0;
		while(in < npix)
		{
			uint count;
	
			// Deal with transparency
			if (hasclrkey && pixels[in] == clrkey) {
				if (runstart >= 0) { // finish normal run
					data[runstart] = out-runstart-1;
					runstart = -1;
				}
					
				count = 1;
				in++;
				while(in < npix && pixels[in] == clrkey) {
					count++;
					in++;
				}
				
				data[out++] = (1<<14) | count;
				continue;
			}
			
			// Deal with shadow
			if (hasshadow && pixels[in] == shadowclr) {
				if (runstart >= 0) { // finish normal run
					data[runstart] = out-runstart-1;
					runstart = -1;
				}
					
				count = 1;
				in++;
				while(in < npix && pixels[in] == shadowclr) {
					count++;
					in++;
				}
				
				data[out++] = (3<<14) | count;
				continue;
   		}
			
			// Check if it's a player color
			if (hasplrclrs) {
				int idx;
				
	         for(idx = 0; idx < hasplrclrs; idx++) {
            	if(pixels[in] == plrclrs[idx])
						break;
				}
				
				if (idx < hasplrclrs) {
					if (runstart >= 0) { // finish normal run
						data[runstart] = out-runstart-1;
						runstart = -1;
					}
					
					count = 1;
					in++;
					while(in < npix && pixels[in] == shadowclr) {
						count++;
						in++;
					}
					
					data[out++] = (2<<14) | count;
					data[out++] = idx;
					continue;
            }
         }
			
			// Normal run of pixels
			if (runstart < 0)
				runstart = out++;
			
			data[out++] = pixels[in++];
      }
   
		if (runstart >= 0) { // finish normal run
			data[runstart] = out-runstart-1;
			runstart = -1;
		}
		
		// Store the frame
		add_pic(out*sizeof(short), data);
	} catch(...) {
		if (data)
			free(data);
		throw;
	}
	
	free(data);
}

//
// this is a function which reads a animation from a file
//
int Animation::read(FileRead* f)
{
   ushort np;
   np = f->Unsigned16();

   uint i;
   
   ushort size;
   for(i=0; i<np; i++) {
      size = f->Unsigned16();
      
      ushort *ptr = (ushort*)f->Data(size);
      add_pic(size, ptr);
   }
  
   return RET_OK;
}

/** Animation::parse(const char *directory, Section *s, const char *picnametempl)
 *
 * Read an animation which sits in the given directory.
 * The animation is described by the given section.
 * picnametempl, when not null, overrides the default logic for determining
 *  picture file names when the file names aren't given explicitly.
 */
void Animation::parse(const char *directory, Section *s, const char *picnametempl)
{
	char templbuf[256]; // used when picnametempl == 0
	char pictempl[256];

	if (!picnametempl) {
		// allow named sections to automatically give sequence names
		snprintf(templbuf, sizeof(templbuf), "%s_??.bmp", s->get_name());
		picnametempl = templbuf;
	}
	
	snprintf(pictempl, sizeof(pictempl), "%s/%s", directory, picnametempl);

	// Get colorkey, shadow color and hotspot
	int r, g, b;
   ushort clrkey, shadowclr;
   r = s->get_int("clrkey_r", 255); 
   g = s->get_int("clrkey_g", 255);
   b = s->get_int("clrkey_b", 255);
   clrkey = pack_rgb(r, g, b);
   r = s->get_int("shadowclr_r", 0);
   g = s->get_int("shadowclr_g", 0);
   b = s->get_int("shadowclr_b", 0);
   shadowclr = pack_rgb(r, g, b);


	// TODO: Frames of varying size / hotspot?

	hsx = s->get_int("hot_spot_x", 0);
	hsy = s->get_int("hot_spot_y", 0);
	
	// Read frames in one by one
	for(;;) {
		char fname[256];
		int nr = npics;
		char *p;
		
		// create the file name by reverse-scanning for '?' and replacing
		strcpy(fname, pictempl);
		p = fname + strlen(fname);
		while(p > fname) {
			if (*--p != '?')
				continue;
			
			*p = '0' + (nr % 10);
			nr = nr / 10;
		}
		
		if (nr) // cycled up to maximum possible frame number
			break;
		
		// is the frame actually there?
		if (!g_fs->FileExists(fname))
			break;
	
		Pic* pic = new Pic();
		if (pic->load(fname)) {
			delete pic;
			break;
		}

		if (!npics) {
			w = pic->get_w();
			h = pic->get_h();
		}
		
		if(pic->get_w() != w && pic->get_h() != h) {
			delete pic;
			throw wexception("%s: frame must be %ix%i pixels big", fname, w, h);
		}
		
		try {
			add_pic(pic, true, clrkey, true, shadowclr, 0, 0);
		} catch(...) {
			delete pic;
			throw;
		}
		
		delete pic;
	}
	
	if (!npics)
		throw wexception("Animation %s has no frames", pictempl);
}

// 
// class Logic_Bob_Descr
//
Logic_Bob_Descr::Logic_Bob_Descr(const char *name)
{
	snprintf(m_name, sizeof(m_name), "%s", name);
}

void Logic_Bob_Descr::read(const char *directory, Section *s)
{
	char picname[256];
	
	snprintf(picname, sizeof(picname), "%s_??.bmp", m_name);
   anim.parse(directory, s, picname);
}

/*
==============================================================================   

class Diminishing_Bob

==============================================================================   
*/

// 
// Description
// 
class Diminishing_Bob_Descr : public Logic_Bob_Descr {
   public:
		Diminishing_Bob_Descr(const char *name);
      virtual ~Diminishing_Bob_Descr(void) { }

      virtual void read(const char *directory, Section *s);
      Map_Object *create_object();

   private:
      Logic_Bob_Descr* ends_in;
      ushort stock;
      uchar occupies;
};

Diminishing_Bob_Descr::Diminishing_Bob_Descr(const char *name)
	: Logic_Bob_Descr(name)
{
	ends_in=0;
	stock=0;
}

void Diminishing_Bob_Descr::read(const char *directory, Section *s)
{
   Logic_Bob_Descr::read(directory, s);
   
   stock = s->get_int("stock", 0);
		
	// TODO
	s->get_string("ends_in", 0);
}


// 
// Implementation
// 
class Diminishing_Bob : public Map_Object {
		MO_DESCR(Diminishing_Bob_Descr);

   public:
		Diminishing_Bob(Diminishing_Bob_Descr* d);
      virtual ~Diminishing_Bob(void);

		virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

Diminishing_Bob::Diminishing_Bob(Diminishing_Bob_Descr* d)
	: Map_Object(d)
{
} 

Diminishing_Bob::~Diminishing_Bob()
{
}

void Diminishing_Bob::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	// infinitely idle
	start_task_idle(g, get_descr()->get_anim(), -1);
}

Map_Object *Diminishing_Bob_Descr::create_object()
{
	return new Diminishing_Bob(this);
}

/*
==============================================================================   

class Boring_Bob

==============================================================================   
*/

// 
// Description
// 
class Boring_Bob_Descr : public Logic_Bob_Descr {
   public:
      Boring_Bob_Descr(const char *name);
      virtual ~Boring_Bob_Descr(void) { } 

      virtual void read(const char *directory, Section *s);
      Map_Object *create_object();

   private:
      ushort ttl; // time to life
      uchar occupies;
};

Boring_Bob_Descr::Boring_Bob_Descr(const char *name)
	: Logic_Bob_Descr(name)
{
	ttl = 0;
}

void Boring_Bob_Descr::read(const char *directory, Section *s)
{
	Logic_Bob_Descr::read(directory, s);

   ttl = s->get_int("life_time", 0);
}


// 
// Implementation
// 
class Boring_Bob : public Map_Object {
		MO_DESCR(Boring_Bob_Descr);

   public:
      Boring_Bob(Boring_Bob_Descr *d);
      virtual ~Boring_Bob(void);

		virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

Boring_Bob::Boring_Bob(Boring_Bob_Descr *d)
	: Map_Object(d)
{
}

Boring_Bob::~Boring_Bob()
{
}

void Boring_Bob::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	// infinitely idle
	start_task_idle(g, get_descr()->get_anim(), -1);
}

Map_Object *Boring_Bob_Descr::create_object()
{
	return new Boring_Bob(this);
}

/*
==============================================================================   

class Critter_Bob

==============================================================================   
*/

// 
// Description
// 
class Critter_Bob_Descr : public Logic_Bob_Descr {
   public:
      Critter_Bob_Descr(const char *name);
      virtual ~Critter_Bob_Descr(void) { } 

      virtual void read(const char *directory, Section *s);
      Map_Object *create_object();

      inline bool is_swimming(void) { return swimming; }
      inline Animation* get_walk_ne_anim(void) { return &walk_ne; }
      inline Animation* get_walk_nw_anim(void) { return &walk_nw; }
      inline Animation* get_walk_se_anim(void) { return &walk_se; }
      inline Animation* get_walk_sw_anim(void) { return &walk_sw; }
      inline Animation* get_walk_w_anim(void) { return &walk_w; }
      inline Animation* get_walk_e_anim(void) { return &walk_e; }

   private:
      ushort stock;
      bool swimming;
      Animation walk_ne;
      Animation walk_nw;
      Animation walk_e;
      Animation walk_w;
      Animation walk_se;
      Animation walk_sw;
};

Critter_Bob_Descr::Critter_Bob_Descr(const char *name)
	: Logic_Bob_Descr(name)
{
	stock = swimming = 0;
}

void Critter_Bob_Descr::read(const char *directory, Section *s)
{
   Logic_Bob_Descr::read(directory, s);

   stock = s->get_int("stock", 0);
   swimming = s->get_bool("swimming", false);

   // read all the other animatins
	char picname[256];
	
	snprintf(picname, sizeof(picname), "%s_walk_ne_??.bmp", m_name);
	walk_ne.parse(directory, s, picname);
	
	snprintf(picname, sizeof(picname), "%s_walk_e_??.bmp", m_name);
	walk_e.parse(directory, s, picname);
	
	snprintf(picname, sizeof(picname), "%s_walk_se_??.bmp", m_name);
	walk_se.parse(directory, s, picname);
	
	snprintf(picname, sizeof(picname), "%s_walk_sw_??.bmp", m_name);
	walk_sw.parse(directory, s, picname);
	
	snprintf(picname, sizeof(picname), "%s_walk_w_??.bmp", m_name);
	walk_w.parse(directory, s, picname);

	snprintf(picname, sizeof(picname), "%s_walk_nw_??.bmp", m_name);
	walk_nw.parse(directory, s, picname);
}


//
// Implementation
//
#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 2000 // wait up to 12 seconds between moves

class Critter_Bob : public Map_Object {
		MO_DESCR(Critter_Bob_Descr);

   public:
      Critter_Bob(Critter_Bob_Descr *d);
      virtual ~Critter_Bob(void);

		uint get_movecaps();

		virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

Critter_Bob::Critter_Bob(Critter_Bob_Descr *d)
	: Map_Object(d)
{
}

Critter_Bob::~Critter_Bob()
{
}

uint Critter_Bob::get_movecaps() { return get_descr()->is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK; }

void Critter_Bob::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	if (prev == TASK_IDLE)
	{
		// Build the animation list - this needs a better solution (AnimationManager, anyone?)
		Animation *anims[6];
		
		anims[0] = get_descr()->get_walk_ne_anim();
		anims[1] = get_descr()->get_walk_e_anim();
		anims[2] = get_descr()->get_walk_se_anim();
		anims[3] = get_descr()->get_walk_sw_anim();
		anims[4] = get_descr()->get_walk_w_anim();
		anims[5] = get_descr()->get_walk_nw_anim();
		
		// Pick a target at random
		Coords dst;
		
		dst.x = m_pos.x + (rand()%5) - 2;
		dst.y = m_pos.y + (rand()%5) - 2;
		
		if (start_task_movepath(g, dst, 3, anims))
			return;
	
		start_task_idle(g, get_descr()->get_anim(), 1 + g->logic_rand()%1000);
		return;
	}
	
	// idle for a longer period
	start_task_idle(g, get_descr()->get_anim(), 1000 + g->logic_rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK);
}

Map_Object *Critter_Bob_Descr::create_object()
{
	return new Critter_Bob(this);
}

/*
==============================================================================   

class Growing_Bob

==============================================================================   
*/

#if 0 // NOT IMPLEMENTED
//
// class Growing_Bob_Descr
//
class Growing_Bob_Descr : public Logic_Bob_Descr {
   public:
      Growing_Bob_Descr(void) { ends_in=0; growing_speed=0; }
      virtual ~Growing_Bob_Descr(void) {  }

      virtual int read(FileRead* f);
      Map_Object *create_object();

   private:
      Logic_Bob_Descr* ends_in;
      ushort growing_speed;
      uchar occupies;
};

int Growing_Bob_Descr::read(FileRead* f)
{
   cerr << "Growing_Bob_Descr::read() TODO!" << endl;
   return RET_OK;
}

Map_Object *Growing_Bob_Descr::create_object()
{
   cerr << "Growing_Bob_Descr::create_object() TODO!" << endl;
	
	return 0; // uh oh
}
#endif


/*
==============================================================================

Logic_Bob factory

==============================================================================
*/

/** Logic_Bob_Descr::create_from_dir(const char *directory) [static]
 *
 * Master factory to read a bob from the given directory and create the
 * appropriate description class.
 *
 * May return 0.
 */
Logic_Bob_Descr *Logic_Bob_Descr::create_from_dir(const char *directory)
{
	const char *name;
	
	// name = last element of path
	const char *slash = strrchr(directory, '/');
	const char *backslash = strrchr(directory, '\\');
	
	if (backslash && (!slash || backslash > slash))
		slash = backslash;
	
	if (slash)
		name = slash+1;
	else
		name = directory;

	// Open the config file
	Logic_Bob_Descr *bob = 0;
	char fname[256];
	
	snprintf(fname, sizeof(fname), "%s/conf", directory);
	
	if (!g_fs->FileExists(fname))
		return 0;
		
	try
	{
		Profile prof(fname, "global"); // section-less file
		Section *s = prof.get_next_section(0);
		assert(s);

		const char *type = s->get_safe_string("type");

		if(!strcasecmp(type, "diminishing")) {
			bob = new Diminishing_Bob_Descr(name);
	/*	} else if(!strcasecmp(type, "growing")) {
			bob = new Growing_Bob_Descr(name);*/
		} else if(!strcasecmp(type, "boring")) {
			bob = new Boring_Bob_Descr(name);
		} else if(!strcasecmp(type, "critter")) {
			bob = new Critter_Bob_Descr(name);
		} else
			throw wexception("Unsupported bob type '%s'", type);

		bob->read(directory, s);
		prof.check_used();
	}
	catch(std::exception &e) {
		if (bob)
			delete bob;
		throw wexception("Error reading bob %s: %s", directory, e.what());
	}
	catch(...) {
		if (bob)
			delete bob;
		throw;
	}
	
	return bob;
}
