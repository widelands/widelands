/*
 * Copyright (C) 2002 by Holger Rapp
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

#include "bob_descr.h"


/*
 * construct a bob picture description (as used for bobs in Datafiles)
 * and adds it to the given BobDescr
 *
 * Args: bob     construct description in this bob description 
 * Returns: used size of data or 0 if function fails
 */
uint Bob_Data_Pic_Descr::construct_description(BobDescr* bob) {
   bob->npics++;
   if(bob->npics==1) {
      bob->pics=(BobPicDescr*) malloc(sizeof(BobPicDescr));
      if(!bob->pics) return 0;
   } else {
      bob->pics=(BobPicDescr*) realloc(bob->pics, sizeof(BobPicDescr)*bob->npics);
      if(!bob->pics) return 0;
   }
  
   BobPicDescr* pic=&bob->pics[bob->npics-1];
   // normally, the compressed data will be slightly smaller, but make things sure here: use 
   // double size!
   pic->data=(char*) malloc(get_w()*get_h()*2*2); 
   uint i=0;
   for(i=0; i<get_w()*get_h()*2*2; i++) {
      pic->data[i]='\0';
   }
   ushort* epixels=(ushort*)pic->data; 
   
   uint n=0;
   uint count=0;
   int lidx=-1;
   uint pixc=0;
   uint npix=get_h()*get_w();
   for(i=0; i<npix; i++) {
      if(has_clrkey() && pixels[i]==get_clrkey()) {
         epixels[n]|=CMD_TRANS;
         count=0;
         while(pixels[i]==get_clrkey() && i<npix) { count++; i++; }
         epixels[n]|=count;
         n++;
         i--;

         // write last count
         if(lidx!=-1) {
         //   cerr << pixc << "x bunte pixel" << endl;
            epixels[lidx]=pixc;
         }
         lidx=-1;
       //  cerr << "Trans: " << count << endl;
      } else if(has_shadow && pixels[i]==shadowclr) {
         epixels[n]|=CMD_SHAD;
         count=0;
         while(pixels[i]==shadowclr && i<npix) { count++; i++; }
         epixels[n]|=count;
         n++;
         i--;
   
         // write last count
         if(lidx!=-1) {
         //   cerr << pixc << "x bunte pixel" << endl;
            epixels[lidx]=pixc;
         }
         lidx=-1;
       //  cerr << "Shadw: " << count << endl;
      } else if(has_plclr) {
         uint idx;
         for(idx=0; idx<4; idx++) {
            if(pixels[i]==play_clr[idx]) {
               epixels[n]|=CMD_PLCLR;
               epixels[n]|=idx;
               count=0;
               n++;
               while(pixels[i]==play_clr[idx] && i<npix) { count++; i++; }
               epixels[n]=count;
               n++;
               i--;
               // write last count
               if(lidx!=-1) {
              //    cerr << pixc << "x bunte pixel" << endl;
                  epixels[lidx]=pixc;
               }
               lidx=-1;
            //   cerr << "PlClr " << idx << ": " << count << endl;
               break;
            }
         }
         if(idx==4) {
            // nothing special about the stuff, just some color pix
            if(lidx==-1) {
               lidx=n;
               pixc=0;
               n++;
            }
            epixels[n]=pixels[i];
            n++;
            pixc++;
         }
      }
   } 
  // cerr << "n ist: " << n << endl;
  
   pic->data=(char*) malloc(n*2);
   pic->size=n*2;
   
   return n*2;
}

void Bob_Data_Pic_Descr::set_shadowclr(ushort clr) { 
   has_shadow=true;
   shadowclr=clr; 
}

void Bob_Data_Pic_Descr::set_plclr(ushort darkest, ushort dark, ushort middle, ushort light) {
   has_plclr=1;
   play_clr[0]=darkest;
   play_clr[1]=dark;
   play_clr[2]=middle;
   play_clr[3]=light;
}


