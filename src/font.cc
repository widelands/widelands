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
#include "font.h"


/** class Font_Handler
 *
 * This class generates font Pictures out of strings and returns them
 *
 * It's a singleton
 *
 * It's a little ugly, since every char is hold in it's own pic which is quite a waste of 
 * good resources
 * 
 * DEPENDS: class	Graph::Pic
 * DEPENDS:	func	Graph::copy_pic
 * DEPENDS: class	myfile
 */

/** Font_Handler::Font_Handler(void)
 *
 * Simple inits
 *
 * Agrs: None
 * Returns: Nothing
 */
Font_Handler::Font_Handler(void) {
		  for (uint i=0; i<MAX_FONTS; i++) {
					 fonts[i].h=0;
		  }
}

/** Font_Handler::~Font_Handler(void) 
 *
 * Simple cleanups
 *
 * Args: None
 * Returns: Nothing
 */
Font_Handler::~Font_Handler(void) {
}

/** int Font_Handler::load_font(const char* str, ushort fn )
 *
 * This registers a certain font with the given
 * objects
 *
 * Args:	str	name of the font we want to load
 * 		fn 	number of font to register
 *	Returns: ERR_FAILED, FERR_INVAL_VERSION, FERR_INVAL_FILE, RET_OK
 */
int Font_Handler::load_font(const char* str, ushort fn)
{
	assert(fn<MAX_FONTS);
	assert(str);

	char buf[200];
	FileRead f;
	
	snprintf(buf, sizeof(buf), "fonts/%s.wff", str);
	f.Open(g_fs, buf);
	
	// TODO: actually use FileRead's nice features for endian safety
	FHeader *fh = (FHeader*)f.Data(sizeof(FHeader));

	if(WLFF_VERSIONMAJOR(fh->version) > WLFF_VERSIONMAJOR(WLFF_VERSION)) {
		return FERR_INVAL_VERSION;
	}
	if(WLFF_VERSIONMAJOR(fh->version) == WLFF_VERSIONMAJOR(WLFF_VERSION)) {
		if(WLFF_VERSIONMINOR(fh->version) > WLFF_VERSIONMINOR(WLFF_VERSION)) {
			return FERR_INVAL_VERSION;
		}
	}

	fonts[fn].h = fh->height;

	uchar c;
	ushort w;
	for(unsigned int i=0; i<96; i++) {
		c = f.Unsigned8();
		if (c != (i+32))
			return FERR_INVAL_FILE;
		
		w = f.Unsigned16();

		ushort *data = (ushort*)f.Data(sizeof(ushort)*w*fonts[fn].h);
		fonts[fn].p[i].create(w, fonts[fn].h, data);
		fonts[fn].p[i].set_clrkey(fh->clrkey);
	}
	
	return RET_OK;
}

/** Pic* Font_Handler::get_string(const uchar* str, const ushort f);
 *
 * This function constructs a Picture containing the given text and
 * returns it. It just makes ONE line. Not a whole paragraph
 *
 * Args:	str	String to construct
 * 		f		Font to use
 * Returns:	Pointer to picture, caller must free it later on
 */
Pic* Font_Handler::get_string(const char* str, const ushort f) {
		  assert(f<MAX_FONTS);

		  char* buf = new char[strlen(str)+1];
		  uchar c;
		  uint n=0;
		  uint x=0;
		  uint w=0;

		  for(uint i=0; i<strlen(str); i++) {
					 c=(uchar) str[i];
					 if(c=='\t' || c=='\r' || c=='\n' || c=='\b' || c=='\a') continue;
					 buf[n]=c;
					 ++n;
					 if(c < 32  || c > 127) {
								// c is NOT an international ASCII char, we skip it silently
								c=127;
					 }
					 w+= fonts[f].p[c-32].get_w();
		  }
		  buf[n]='\0';

		  // Now buf contains only valid chars
		  Pic* retval=new Pic;
		  retval->set_clrkey(fonts[f].p[0].get_clrkey());
		  retval->set_size(w, fonts[f].h);

		  for(uint j=0; j<strlen(buf); j++) {
					 c=buf[j];
					 if(c < 32  || c > 127) {
								// c is NOT an international ASCII char, we skip it silently
								c=127;
					 }

					 // change c, so we get correct offsets in our font file
					 c-=32;

					 copy_pic(retval, &fonts[f].p[c], x, 0, 0, 0, fonts[f].p[c].get_w(), fonts[f].h);
					 x+=fonts[f].p[c].get_w();
		  }

		  delete[] buf;
		  return retval;
}
