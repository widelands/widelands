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

#include "bob_descr.h"
#include "string.h"
#include "wtfcreate.h"
#include "../src/graphic.h"


/** class Bob_Data_Pic_Descr 
 *
 * This class extends class Pic and can add itself to a bob information, 
 * it's only used by Bob_Descr
 */
class Bob_Data_Pic_Descr : public Pic {
   public:
      Bob_Data_Pic_Descr(void) : Pic() { has_plclr=0; has_shadow=0; shadowclr=0; }
      ~Bob_Data_Pic_Descr(void) { }

      void set_shadowclr(ushort clr); 
      void set_plclr(ushort darkest, ushort dark, ushort middle, ushort light);

      uint construct_description(Bob_Descr*);
      void write(Binary_file* f);

   private:
      ushort shadowclr;
      ushort play_clr[4];
      bool has_shadow;
      bool has_plclr;
};

/*
 * construct a bob picture description (as used for bobs in Datafiles)
 * and writes it to the given file
 */
void Bob_Data_Pic_Descr::write(Binary_file* f) {

   cerr << "Bob_Data_Pix_Descr::write()!! TODO!" << endl;

   /*   bob->npics++;
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
   
   return n*2;*/
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


//
// class Bob_Descr!
//


// 
// This function loads a bob from several files and constructs it into a BobDescr
//
uint Bob_Descr::construct(const char* pfile_names, const char* dirname, const char* subdir, ushort clrkey, ushort shadowclr, ushort* w, ushort *h) {

   npics=0;
   *w=0;
   *h=0;

   uint len=strlen(dirname)+strlen(pfile_names)+1;
   if(subdir) len+=strlen(subdir)+1;
   char* buf = new char[len];
   strcpy(buf, dirname);
   if(subdir) { 
      strcat(buf, subdir); 
      if(buf[strlen(buf)-1]!= SEPERATOR) {
         buf[strlen(buf)+1]='\0'; 
         buf[strlen(buf)]=SEPERATOR; 
      }
   }
   strcat(buf, pfile_names);

   uint nidx=strlen(buf);
   while(!(buf[nidx]=='?' && buf[nidx+1]=='?') && nidx) nidx--;
   if(buf[nidx]!='?' && buf[nidx+1]!='?') {
      delete[] buf;
      return ERR_INVAL_FILE_NAMES; 
   }

   char one, ten;
   Bob_Data_Pic_Descr* p;
   for(ten='0'; ten<='9'; ten++) {
      buf[nidx]=ten;
      for(one='0'; one<='9'; one++) {
         buf[nidx+1]=one;

          // cerr << buf << endl;

         // Handle this picture!
         p=new Bob_Data_Pic_Descr();
         if(p->load(buf)) {
            delete p;
            break; // file not found or some error
         }
         
         if(!(*w) && !(*h)) {
            *w=p->get_w();
            *h=p->get_h();
         } else {
            if(*w!=p->get_w() || *h!=p->get_h()) {
               delete p;
               delete[] buf;
               return ERR_INVAL_DIMENSIONS;
            }
         }
         npics++;
         p->set_clrkey(clrkey);
         p->set_shadowclr(shadowclr);
         p->set_plclr(DARKEST_PL_CLR, DARK_PL_CLR, MIDDLE_PL_CLR, LIGHT_PL_CLR);
   //      p->construct_description(bob);

         delete p;
      }
      if((one-1)!='9') break;
   }

   if(!npics) {
      delete[] buf;
      return ERR_NOPICS;
   }
   
   delete[] buf;
   return OK;
}

/*
 * this function writes a bob description into a file
 */
void Bob_Descr::write(Binary_file* f) {

   cerr << "Bob_Descr::write()! TODO!" << endl;
}

