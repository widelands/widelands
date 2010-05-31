/*
 * Copyright 2010 by the Widelands Development Team
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
 */

#include "graphic/graphic.h"
#include "surface_opengl.h"

#include "log.h"
#include "upcast.h"
#include "wexception.h"

#include <SDL.h>


#ifdef USE_OPENGL

/*
 * Updating the whole Surface
 */
void SurfaceOpenGL::update() {
	assert(g_opengl);
	SDL_GL_SwapBuffers();
}

/*
===============
Draws the outline of a rectangle
===============
*/
void SurfaceOpenGL::draw_rect(const Rect rc, const RGBColor clr) {
	assert(g_opengl);
	//log("SurfaceOpenGL::draw_rect() for opengl is experimental\n");
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINE_LOOP);
		glColor3ub(clr.r(), clr.g(), clr.b());
		glVertex2f(m_offsx + rc.x + 0.5f,        m_offsy + rc.y + 0.5f);
		glVertex2f(m_offsx + rc.x + rc.w - 0.5f, m_offsy + rc.y + 0.5f);
		glVertex2f(m_offsx + rc.x + rc.w - 0.5f, m_offsy + rc.y + rc.h - 0.5f);
		glVertex2f(m_offsx + rc.x + 0.5f,        m_offsy + rc.y + rc.h - 0.5f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}


/*
===============
Draws a filled rectangle
===============
*/
void SurfaceOpenGL::fill_rect(const Rect rc, const RGBAColor clr) {
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 0);
	assert(rc.h >= 0);
	assert(g_opengl);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glBegin(GL_QUADS);
		glColor4ub(clr.r, clr.g, clr.b, clr.a);
		glVertex2f(m_offsx + rc.x,        m_offsy + rc.y);
		glVertex2f(m_offsx + rc.x + rc.w, m_offsy + rc.y);
		glVertex2f(m_offsx + rc.x + rc.w, m_offsy + rc.y + rc.h);
		glVertex2f(m_offsx + rc.x,        m_offsy + rc.y + rc.h);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

/*
===============
Change the brightness of the given rectangle
This function is slow as hell.

* This function is a possible point to optimize on
  slow system. It takes a lot of cpu time atm and is
  not needed. It is used by the ui_basic stuff to
  highlight things.
===============
*/
void SurfaceOpenGL::brighten_rect(const Rect rc, const int32_t factor) {
	if (!factor)
		return;

	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);
	assert(g_opengl);

	/* glBlendFunc is a very nice feature of opengl. You can specify how the
	* color is calculated.
	*
	* glBlendFunc(GL_ONE, GL_ONE) means the following:
	* Rnew = Rdest + Rsrc
	* Gnew = Gdest + Gsrc
	* Bnew = Bdest + Bsrc
	* Anew = Adest + Asrc
	* where Xnew is the new calculated color for destination, Xdest is the old
	* color of the destination and Xsrc is the color of the source.
	*/
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_ONE, GL_ONE);

	// And now simply draw a rect with facor as the color
	// (this is the source color) over the region
	glBegin(GL_QUADS);
		glColor3f
			((static_cast<GLfloat>(factor) / 256.0f),
			 (static_cast<GLfloat>(factor) / 256.0f),
			 (static_cast<GLfloat>(factor) / 256.0f));
		glVertex2f(m_offsx + rc.x,        m_offsy + rc.y);
		glVertex2f(m_offsx + rc.x + rc.w, m_offsy + rc.y);
		glVertex2f(m_offsx + rc.x + rc.w, m_offsy + rc.y + rc.h);
		glVertex2f(m_offsx + rc.x,        m_offsy + rc.y + rc.h);
	glEnd();
}

void SurfaceOpenGL::draw_line
	(int32_t const x1, int32_t const y1,
	 int32_t const x2, int32_t const y2, RGBColor const color)
{
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glColor3ub(color.r(), color.g(), color.b());
	glVertex2f(x1 + m_offsx + 0.5f, y1 + m_offsy + 0.5f);
	glVertex2f(x2 + m_offsx + 0.5f, y2 + m_offsy + 0.5f);
	glEnd();
}

/*
===============
Clear the entire bitmap to black
===============
*/
void SurfaceOpenGL::clear()
{
	assert(g_opengl);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	return;
}


void SurfaceOpenGL::blit
	(Point const dst, Surface * const src, Rect const srcrc, bool enable_alpha)
{
	blit(Rect(dst, srcrc.w, srcrc.h), src, srcrc, enable_alpha);
}


void SurfaceOpenGL::blit
	(Rect dst, Surface * src, Rect srcrc, bool enable_alpha)
{
	upcast(SurfaceOpenGL, oglsrc, src);

	assert(g_opengl);
#ifdef DEBUG
	if (m_surf_type != SURFACE_SCREEN)
	{
		throw wexception
			("Surface not the screen: Offscreen rendering not possible");
		return;
	}

	if (not oglsrc)
	{
		throw wexception("Invalid surface: Not a opengl surface");
	}
#endif
	GLuint tex;

	try {
		tex = oglsrc->get_texture();
	} catch (...) {
		throw wexception
			("SurfaceOpenGL::blit(): Source surface has no texture\n");
		return;
	}

	/* Set a texture scaling factor. Normaly texture coordiantes
	* (see glBegin()...glEnd() Block below) are given in the range 0-1
	* to avoid the calculation (and let opengl do it) the texture
	* space is modified. glMatrixMode select which matrix to manipulate
	* (the texture transformation matrix in this case). glLoadIdentity()
	* resets the (selected) matrix to the identity matrix. And finally
	* glScalef() calculates the texture matrix.
	*/
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef
		(1.0f / static_cast<GLfloat>(oglsrc->get_tex_w()),
		 1.0f / static_cast<GLfloat>(oglsrc->get_tex_h()), 1);

	// Enable Alpha blending
	if (enable_alpha) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else
		glDisable(GL_BLEND);

	/* select the texture to paint on the screen
	* openGL does not know anything about SDL_Surfaces
	* opengl uses textures to handle images
	* getTexture() returns the texture id of the Surface. It creates
	* the texture from the SDL_Surface if it doesn't exist
	*/
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	/* This block between blBegin() and glEnd() does the blit.
	* It draws a textured rectangle. glTexCoord2i() set the Texture
	* Texture cooardinates. This is the source rectangle.
	* glVertex2f() sets the screen coordiantes which belong to the
	* previous texture coordinate. This is the destination rectangle
	*/

	glBegin(GL_QUADS);
		//set color white, otherwise textures get mixed with color
		glColor3f(1.0, 1.0, 1.0);
		//top-left
		glTexCoord2i(srcrc.x,           srcrc.y);
		glVertex2i  (m_offsx + dst.x,   m_offsy + dst.y);
		//top-right
		glTexCoord2i(srcrc.x + srcrc.w, srcrc.y);
		glVertex2f  (m_offsx + dst.x + dst.w,  m_offsy + dst.y);
		//botton-right
		glTexCoord2i(srcrc.x + srcrc.w, srcrc.y + srcrc.h);
		glVertex2f  (m_offsx + dst.x + dst.w,  m_offsy + dst.y + dst.h);
		//bottom-left
		glTexCoord2i(srcrc.x,           srcrc.y + srcrc.h);
		glVertex2f  (m_offsx + dst.x,   m_offsy + dst.y + dst.h);
	glEnd();

	glLoadIdentity();
}

#endif //USE_OPENGL
