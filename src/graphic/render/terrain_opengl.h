/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifdef USE_OPENGL

#include "surface_opengl.h"
#include "vertex.h"


void draw_field_opengl
	(Surface & surf,
	 Rect const & subwin,
	 Vertex const & p1,
	 Vertex const & p2,
	 Vertex const & p3,
	 Texture const & texture)
{
	if(p1.b<=-128 and p2.b<=-128 and p3.b<=-128)
		return;

#warning This code works for the main view but is broken for the watchwindow

	glBindTexture( GL_TEXTURE_2D, texture.getTexture());
	GLfloat brightness;

	Vertex t1(p1), t2(p2), t3(p3);
	
	t1.x = p1.x + subwin.x;
	t1.y = p1.y + subwin.y;
	t2.x = p2.x + subwin.x;
	t2.y = p2.y + subwin.y;
	t3.x = p3.x + subwin.x;
	t3.y = p3.y + subwin.y;

	if ( (t1.x<subwin.x and t2.x<subwin.x and t3.x<subwin.x) or
		(t1.y<subwin.y and t2.y<subwin.y and t3.y<subwin.y))
		return;

	int subxr = subwin.x + subwin.w, subyd = subwin.y + subwin.h;
	if ( (t1.x > subxr and t2.x>subxr and t3.x>subxr) or
		(t1.y>subyd and t2.y>subyd and t3.y>subyd))
		return;

	glBegin(GL_TRIANGLES);
		brightness=(150.0+p1.b)/150.0;
		glColor3f(brightness, brightness, brightness);
		glTexCoord2i(t1.tx, t1.ty);
		glVertex2f(t1.x, t1.y);

		brightness=(150.0+p2.b)/150.0;
		glColor3f(brightness, brightness, brightness);
		glTexCoord2i(t2.tx, t2.ty);
		glVertex2f(t2.x, t2.y);

		brightness=(150.0+p3.b)/150.0;
		glColor3f(brightness, brightness, brightness);
		glTexCoord2i(t3.tx, t3.ty);
		glVertex2f(t3.x, t3.y);
	glEnd();
}

#define horiz_road_opengl(tex, start, end)                                    \
	glBindTexture( GL_TEXTURE_2D, tex);                                        \
	glBegin(GL_QUADS);                                                         \
		glTexCoord2i(0, 0);                                                     \
		glVertex2f(subwin.x + start.x, subwin.y + start.y - 2);                 \
		glTexCoord2i(TEXTURE_WIDTH, 0);                                         \
		glVertex2f(subwin.x + end.x,   subwin.y + end.y - 2);                   \
		glTexCoord2i(TEXTURE_WIDTH, 4);                                         \
		glVertex2f(subwin.x + end.x,   subwin.y + end.y + 2);                   \
		glTexCoord2i(TEXTURE_WIDTH, 4);                                         \
		glVertex2f(subwin.x + start.x, subwin.y + start.y + 2);                 \
	glEnd();

#define vert_road_opengl(tex, start, end)                                     \
	glBindTexture( GL_TEXTURE_2D, tex);                                        \
	glBegin(GL_QUADS);                                                         \
		glTexCoord2i(0, 0);                                                     \
		glVertex2f(subwin.x + start.x - 3, subwin.y + start.y);                 \
		glTexCoord2i(6, 0);                                                     \
		glVertex2f(subwin.x + start.x + 3, subwin.y + start.y);                 \
		glTexCoord2i(TEXTURE_WIDTH, TEXTURE_HEIGHT);                            \
		glVertex2f(subwin.x + end.x + 3,   subwin.y + end.y);                   \
		glTexCoord2i(TEXTURE_WIDTH - 6 , TEXTURE_HEIGHT);                       \
		glVertex2f(subwin.x + end.x - 3,   subwin.y + end.y);                   \
	glEnd();

void draw_roads_opengl
	(Rect & subwin,
	 uint8_t roads,
	 Vertex const & f_vert,
	 Vertex const & r_vert,
	 Vertex const & bl_vert,
	 Vertex const & br_vert)
{
	uint8_t road;

	GLuint rt_normal =
		dynamic_cast<SurfaceOpenGL const &>
		(*g_gr->get_road_texture(Widelands::Road_Normal)).get_texture();
	GLuint rt_busy   =
		dynamic_cast<SurfaceOpenGL const &>
		(*g_gr->get_road_texture(Widelands::Road_Busy)).get_texture();

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 0.6f);

	road = (roads >> Widelands::Road_East) & Widelands::Road_Mask;
	if ((-128 < f_vert.b or -128 < r_vert.b) and road) {
		switch (road) {
		case Widelands::Road_Normal:
			horiz_road_opengl(rt_normal, f_vert, r_vert)
			break;
		case Widelands::Road_Busy:
			horiz_road_opengl(rt_busy, f_vert, r_vert)
			break;
		default:
			assert(false);
		}
	}

	road = (roads >> Widelands::Road_SouthEast) & Widelands::Road_Mask;
	if ((-128 < f_vert.b or -128 < br_vert.b) and road) {
		switch (road) {
		case Widelands::Road_Normal:
			vert_road_opengl(rt_normal, f_vert, br_vert);
			break;
		case Widelands::Road_Busy:
			vert_road_opengl(rt_busy, f_vert, br_vert);
			break;
		default:
			assert(false);
		}
	}

	road = (roads >> Widelands::Road_SouthWest) & Widelands::Road_Mask;
	if ((-128 < f_vert.b or -128 < bl_vert.b) and road) {
		switch (road) {
		case Widelands::Road_Normal:
			vert_road_opengl(rt_normal, f_vert, bl_vert);
			break;
		case Widelands::Road_Busy:
			vert_road_opengl(rt_busy, f_vert, bl_vert);
			break;
		default:
			assert(false);
		}
	}
}

#endif
