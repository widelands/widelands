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

//
// this is a function which reads a animation from a file
//
int Animation::read(Binary_file* f) {
   ushort np;
   f->read(&np, sizeof(ushort));

   uint i;
   char* buf;
   ushort buf_size=15000;
   buf=(char*) malloc(buf_size);
   
   ushort size;
   for(i=0; i<np; i++) {
      f->read(&size, sizeof(ushort));
      
      if(size > buf_size) {
         buf_size=size;
         buf=(char*) realloc(buf, buf_size);
      }
      f->read(buf, size);
      add_pic(size, (ushort*) buf);
   }
  
   free(buf);
   
   return RET_OK;
}
   
// 
// class Diminishing_Bob
//
int Diminishing_Bob::act(Game* g) {
   
   // Animate
   pic_idx++;
   if(pic_idx==descr->get_anim()->get_npics()) pic_idx=0;
   cur_pic=descr->get_anim()->get_pic(pic_idx);

   return 1; // next frame again
}

//
// class Boring_Bob
// 
int Boring_Bob::act(Game* g) {
   // Animate
   pic_idx++;
   if(pic_idx==descr->get_anim()->get_npics()) pic_idx=0;
   cur_pic=descr->get_anim()->get_pic(pic_idx);

   return 1; // next frame again
}

//
// class Boring_Bob
// 
int Critter_Bob::act(Game* g) {
   Field* f1;
   int x1, y1, x, y;
   
   switch(state) {
      case IDLE:
         switch((1+(rand() % 10))) {
            default:   
               // keep idle for some frames
               // Animate
               pic_idx++;
               if(pic_idx==descr->get_anim()->get_npics()) pic_idx=0;
               cur_pic=descr->get_anim()->get_pic(pic_idx);
               return (1+(rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK));
               

           case 4:
               // walk e
               if(!descr->is_swimming() && (field->get_terr()->get_is() & TERRAIN_UNPASSABLE)) return 1; // can't walk there, try again
               if(descr->is_swimming() && !(field->get_terr()->get_is() & TERRAIN_WATER)) return 1; // can't walk there, try again
               g->get_map()->get_rn(px, py, field, &x, &y, &f1);
               x1=px; 
               y1=py;
               px=x;
               py=y;
               f1->hook_inst_link(field->unhook_inst_link(this));
               field=f1;
               g->get_map()->get_pix(px, py, &x, &y);
               g->get_map()->get_pix(x1, y1, &x1, &y1);
               vx=(((float) (x-x1))/CRITTER_WALKING_SPEED); // should be ready in so many frames
               vy=(((float) (y-y1))/CRITTER_WALKING_SPEED);
               rel_pos_x=(x1-x);
               rel_pos_y=(y1-y);
               steps=CRITTER_WALKING_SPEED;
               pic_idx=0;
               cur_pic=descr->get_walk_e_anim()->get_pic(0);
               state=WALK_E;
               return 1;
          case 5:
               // walk se
               if(!descr->is_swimming() && (field->get_terr()->get_is() & TERRAIN_UNPASSABLE)) return 1; // can't walk there, try again
               if(descr->is_swimming() && !(field->get_terr()->get_is() & TERRAIN_WATER)) return 1; // can't walk there, try again
               g->get_map()->get_brn(px, py, field, &x, &y, &f1);
               x1=px; 
               y1=py;
               px=x;
               py=y;
               f1->hook_inst_link(field->unhook_inst_link(this));
               field=f1;
               g->get_map()->get_pix(px, py, &x, &y);
               g->get_map()->get_pix(x1, y1, &x1, &y1);
               vx=(((float) (x-x1))/CRITTER_WALKING_SPEED); // should be ready in so many frames
               vy=(((float) (y-y1))/CRITTER_WALKING_SPEED);
               rel_pos_x=(x1-x);
               rel_pos_y=(y1-y);
               steps=CRITTER_WALKING_SPEED;
               pic_idx=0;
               cur_pic=descr->get_walk_se_anim()->get_pic(0);
               state=WALK_SE;
               return 1;
          case 6:
               // walk sw
               if(!descr->is_swimming() && (field->get_terd()->get_is() & TERRAIN_UNPASSABLE)) return 1; // can't walk there, try again
               if(descr->is_swimming() && !(field->get_terd()->get_is() & TERRAIN_WATER)) return 1; // can't walk there, try again
               g->get_map()->get_bln(px, py, field, &x, &y, &f1);
               x1=px; 
               y1=py;
               px=x;
               py=y;
               f1->hook_inst_link(field->unhook_inst_link(this));
               field=f1;
               g->get_map()->get_pix(px, py, &x, &y);
               g->get_map()->get_pix(x1, y1, &x1, &y1);
               vx=(((float) (x-x1))/CRITTER_WALKING_SPEED); // should be ready in so many frames
               vy=(((float) (y-y1))/CRITTER_WALKING_SPEED);
               rel_pos_x=(x1-x);
               rel_pos_y=(y1-y);
               steps=CRITTER_WALKING_SPEED;
               pic_idx=0;
               cur_pic=descr->get_walk_sw_anim()->get_pic(0);
               state=WALK_SW;
               return 1;
          case 7:
               // walk ne
               g->get_map()->get_trn(px, py, field, &x1, &y1, &f1);
               if(!descr->is_swimming() && (f1->get_terd()->get_is() & TERRAIN_UNPASSABLE)) return 1; // can't walk there, try again
               if(descr->is_swimming() && !(f1->get_terd()->get_is() & TERRAIN_WATER)) return 1; // can't walk there, try again
               g->get_map()->get_pix(px, py, &x, &y);
               g->get_map()->get_pix(x1, y1, &x1, &y1);
               vx=(((float) (x1-x))/CRITTER_WALKING_SPEED); // should be ready in so many frames
               vy=(((float) (y1-y))/CRITTER_WALKING_SPEED);
               steps=CRITTER_WALKING_SPEED;
               pic_idx=0;
               cur_pic=descr->get_walk_ne_anim()->get_pic(0);
               state=WALK_NE;
              return 1;
            case 8:
               // walk nw
               g->get_map()->get_tln(px, py, field, &x1, &y1, &f1);
               if(!descr->is_swimming() && (f1->get_terr()->get_is() & TERRAIN_UNPASSABLE)) return 1; // can't walk there, try again
               if(descr->is_swimming() && !(f1->get_terr()->get_is() & TERRAIN_WATER)) return 1; // can't walk there, try again
               g->get_map()->get_pix(px, py, &x, &y);
               g->get_map()->get_pix(x1, y1, &x1, &y1);
               vx=(((float) (x1-x))/CRITTER_WALKING_SPEED); // should be ready in so many frames
               vy=(((float) (y1-y))/CRITTER_WALKING_SPEED);
               steps=CRITTER_WALKING_SPEED;
               pic_idx=0;
               cur_pic=descr->get_walk_nw_anim()->get_pic(0);
               state=WALK_NW;
               return 1;
            case 9:
               // walk w
               g->get_map()->get_ln(px, py, field, &x1, &y1, &f1);
               if(!descr->is_swimming() && (f1->get_terr()->get_is() & TERRAIN_UNPASSABLE)) return 1; // can't walk there, try again
               if(descr->is_swimming() && !(f1->get_terr()->get_is() & TERRAIN_WATER)) return 1; // can't walk there, try again
               g->get_map()->get_pix(px, py, &x, &y);
               g->get_map()->get_pix(x1, y1, &x1, &y1);
               vx=(((float) (x1-x))/CRITTER_WALKING_SPEED); // should be ready in so many frames
               vy=(((float) (y1-y))/CRITTER_WALKING_SPEED);
               steps=CRITTER_WALKING_SPEED;
               pic_idx=0;
               cur_pic=descr->get_walk_w_anim()->get_pic(0);
               state=WALK_W;
               return 1;
        }
         break;

      case WALK_NE:
         pic_idx++;
         if(pic_idx==descr->get_walk_ne_anim()->get_npics()) pic_idx=0;
         cur_pic=descr->get_walk_ne_anim()->get_pic(pic_idx);
         if(steps) {
            steps--;
            rel_pos_x+=vx;
            rel_pos_y+=vy;
         } else {
            g->get_map()->get_trn(px, py, field, &x1, &y1, &f1);
            px=x1;
            py=y1;
            f1->hook_inst_link(field->unhook_inst_link(this));
            field=f1;
            rel_pos_x=0;
            rel_pos_y=0;
            state=IDLE;
            pic_idx=0;
            cur_pic=descr->get_anim()->get_pic(0);
            return (1+(rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK));
         }
         break;
      case WALK_NW:
         pic_idx++;
         if(pic_idx==descr->get_walk_nw_anim()->get_npics()) pic_idx=0;
         cur_pic=descr->get_walk_nw_anim()->get_pic(pic_idx);
         if(steps) {
            steps--;
            rel_pos_x+=vx;
            rel_pos_y+=vy;
         } else {
            g->get_map()->get_tln(px, py, field, &x1, &y1, &f1);
            px=x1;
            py=y1;
            f1->hook_inst_link(field->unhook_inst_link(this));
            field=f1;
            rel_pos_x=0;
            rel_pos_y=0;
            state=IDLE;
            pic_idx=0;
            cur_pic=descr->get_anim()->get_pic(0);
            return (1+(rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK));
         }
         break;
      case WALK_W:
         pic_idx++;
         if(pic_idx==descr->get_walk_w_anim()->get_npics()) pic_idx=0;
         cur_pic=descr->get_walk_w_anim()->get_pic(pic_idx);
         if(steps) {
            steps--;
            rel_pos_x+=vx;
            rel_pos_y+=vy;
         } else {
            g->get_map()->get_ln(px, py, field, &x1, &y1, &f1);
            px=x1;
            py=y1;
            f1->hook_inst_link(field->unhook_inst_link(this));
            field=f1;
            rel_pos_x=0;
            rel_pos_y=0;
            state=IDLE;
            pic_idx=0;
            cur_pic=descr->get_anim()->get_pic(0);
            return (1+(rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK));
         }
         break;
      case WALK_E:
         pic_idx++;
         if(pic_idx==descr->get_walk_e_anim()->get_npics()) pic_idx=0;
         cur_pic=descr->get_walk_e_anim()->get_pic(pic_idx);
         if(steps) {
            steps--;
            rel_pos_x+=vx;
            rel_pos_y+=vy;
         } else {
            rel_pos_x=0;
            rel_pos_y=0;
            state=IDLE;
            pic_idx=0;
            cur_pic=descr->get_anim()->get_pic(0);
            return (1+(rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK));
         }
         break;
      case WALK_SE:
         pic_idx++;
         if(pic_idx==descr->get_walk_se_anim()->get_npics()) pic_idx=0;
         cur_pic=descr->get_walk_se_anim()->get_pic(pic_idx);
         if(steps) {
            steps--;
            rel_pos_x+=vx;
            rel_pos_y+=vy;
         } else {
            rel_pos_x=0;
            rel_pos_y=0;
            state=IDLE;
            pic_idx=0;
            cur_pic=descr->get_anim()->get_pic(0);
            return (1+(rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK));
         }
         break;
      case WALK_SW:
         pic_idx++;
         if(pic_idx==descr->get_walk_sw_anim()->get_npics()) pic_idx=0;
         cur_pic=descr->get_walk_sw_anim()->get_pic(pic_idx);
         if(steps) {
            steps--;
            rel_pos_x+=vx;
            rel_pos_y+=vy;
         } else {
            rel_pos_x=0;
            rel_pos_y=0;
            state=IDLE;
            pic_idx=0;
            cur_pic=descr->get_anim()->get_pic(0);
            return (1+(rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK));
         }
         break;
   } 
   return 1; // next frame again
}

// DOWN HERE: DECRIPTION CLASSES
// 
// class Logic_Bob_Descr
//
int Logic_Bob_Descr::read(Binary_file* f) {
   f->read(name, 30);

   ushort h, w, hsx, hsy;

   f->read(&w, sizeof(ushort));
   f->read(&h, sizeof(ushort));
   f->read(&hsx, sizeof(ushort));
   f->read(&hsy, sizeof(ushort));

   anim.set_dimensions(w,h);
   anim.set_hotspot(hsx, hsy);

   anim.read(f);


   return RET_OK;
}

//
// class Boring_Bob_Descr
//
int Boring_Bob_Descr::read(Binary_file* f) {
   Logic_Bob_Descr::read(f);

   f->read(&ttl, sizeof(ushort));
      
   return RET_OK;
}
int Boring_Bob_Descr::create_instance(Instance* inst) {
   
   inst->obj=new Boring_Bob(this);

//   cerr << "Boring_Bob_Descr::create_instance() TODO!" << endl;

   if(this->anim.get_npics() > 1) return 1; // we have to animate
   return -1; // this thing doesn't animate, so don't waste cycles on it
}

//
// class Critter_Bob_Descr
//
int Critter_Bob_Descr::read(Binary_file* f) {
   Logic_Bob_Descr::read(f);

   f->read(&stock, sizeof(ushort));
   uchar temp;
   f->read(&temp, sizeof(uchar));
   swimming=temp;

   // read all the other bobs
   walk_ne.set_dimensions(anim.get_w(), anim.get_h());
   walk_nw.set_dimensions(anim.get_w(), anim.get_h());
   walk_w.set_dimensions(anim.get_w(), anim.get_h());
   walk_sw.set_dimensions(anim.get_w(), anim.get_h());
   walk_se.set_dimensions(anim.get_w(), anim.get_h());
   walk_e.set_dimensions(anim.get_w(), anim.get_h());
   walk_ne.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_nw.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_w.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_sw.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_se.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_e.set_hotspot(anim.get_hsx(), anim.get_hsy());
   
   walk_ne.read(f);
   walk_e.read(f);
   walk_se.read(f);
   walk_sw.read(f);
   walk_w.read(f);
   walk_nw.read(f);

   return RET_OK;
}

int Critter_Bob_Descr::create_instance(Instance* inst) {
   
   inst->obj=new Critter_Bob(this);

   return 1; // we have (at) least to move 
}

//
// class Diminishing_Bob_Descr
//
int Diminishing_Bob_Descr::read(Binary_file* f) {
   Logic_Bob_Descr::read(f);
   
   f->read(&stock, sizeof(ushort));
   // TODO: ends in should be changed!
   char buf[30];
   f->read(buf, 30);
   
   return RET_OK;
}
int Diminishing_Bob_Descr::create_instance(Instance* inst) {
   inst->obj=new Diminishing_Bob(this);
   
//   cerr << "Diminishing_Bob_Descr::create_instance() TODO!" << endl;
   
   if(this->anim.get_npics() > 1) return 1; // we have to animate
   return -1; // this thing doesn't animate, so don't waste cycles on it
}

//
// class Growing_Bob_Descr
//
int Growing_Bob_Descr::read(Binary_file* f) {
   cerr << "Growing_Bob_Descr::read() TODO!" << endl;
   return RET_OK;
}
int Growing_Bob_Descr::create_instance(Instance* inst) {
   cerr << "Growing_Bob_Descr::create_instance() TODO!" << endl;

   return RET_OK;
}
//
// class Boring_Bob_Descr
//
/*int Critter_Bob_Descr::read(Binary_file* f) {
   cerr << "Critter_Bob_Descr::read() TODO!" << endl;
   return RET_OK;
}*/


