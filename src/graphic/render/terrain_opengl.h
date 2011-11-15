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
#ifndef TERRAIN_OPENGL_H
#define TERRAIN_OPENGL_H

#include "gl_picture_texture.h"
#include "vertex.h"


void draw_field_opengl
	(Rect const & subwin,
	 Vertex const & p1,
	 Vertex const & p2,
	 Vertex const & p3,
	 Texture const & texture)
{
	if (p1.b <= -128 and p2.b <= -128 and p3.b <= -128)
		return;

	glBindTexture(GL_TEXTURE_2D, texture.getTexture());

	Vertex t1(p1), t2(p2), t3(p3);

	t1.x = p1.x + subwin.x;
	t1.y = p1.y + subwin.y;
	t2.x = p2.x + subwin.x;
	t2.y = p2.y + subwin.y;
	t3.x = p3.x + subwin.x;
	t3.y = p3.y + subwin.y;

	if (t1.x < subwin.x and t2.x < subwin.x and t3.x < subwin.x)
		return;
	if (t1.y < subwin.y and t2.y < subwin.y and t3.y < subwin.y)
		return;

	{
		int const subxr = subwin.x + subwin.w;
		if (t1.x > subxr and t2.x > subxr and t3.x > subxr)
			return;
	}
	{
		int const subyd = subwin.y + subwin.h;
		if (t1.y > subyd and t2.y > subyd and t3.y > subyd)
			return;
	}

	glBegin(GL_TRIANGLES); {
		{
			GLfloat const brightness = (150.0 + p1.b) / 150.0;
			glColor3f(brightness, brightness, brightness);
		}
		glTexCoord2i(t1.tx, t1.ty);
		glVertex2f(t1.x, t1.y);

		{
			GLfloat const brightness = (150.0 + p2.b) / 150.0;
			glColor3f(brightness, brightness, brightness);
		}
		glTexCoord2i(t2.tx, t2.ty);
		glVertex2f(t2.x, t2.y);

		{
			GLfloat const brightness = (150.0 + p3.b) / 150.0;
			glColor3f(brightness, brightness, brightness);
		}
		glTexCoord2i(t3.tx, t3.ty);
		glVertex2f(t3.x, t3.y);
	} glEnd();
}

#define horiz_road_opengl(tex, start, end)                                    \
   glBindTexture( GL_TEXTURE_2D, tex);                                        \
   glBegin(GL_QUADS); {                                                       \
      glTexCoord2i(0, 0);                                                     \
      glVertex2f(subwin.x + start.x, subwin.y + start.y - 2);                 \
      glTexCoord2i(TEXTURE_WIDTH, 0);                                         \
      glVertex2f(subwin.x + end.x,   subwin.y + end.y - 2);                   \
      glTexCoord2i(TEXTURE_WIDTH, 4);                                         \
      glVertex2f(subwin.x + end.x,   subwin.y + end.y + 2);                   \
      glTexCoord2i(TEXTURE_WIDTH, 4);                                         \
      glVertex2f(subwin.x + start.x, subwin.y + start.y + 2);                 \
   } glEnd();                                                                 \

#define vert_road_opengl(tex, start, end)                                     \
   glBindTexture( GL_TEXTURE_2D, tex);                                        \
   glBegin(GL_QUADS); {                                                       \
      glTexCoord2i(0, 0);                                                     \
      glVertex2f(subwin.x + start.x - 3, subwin.y + start.y);                 \
      glTexCoord2i(6, 0);                                                     \
      glVertex2f(subwin.x + start.x + 3, subwin.y + start.y);                 \
      glTexCoord2i(TEXTURE_WIDTH, TEXTURE_HEIGHT);                            \
      glVertex2f(subwin.x + end.x + 3,   subwin.y + end.y);                   \
      glTexCoord2i(TEXTURE_WIDTH - 6, TEXTURE_HEIGHT);                        \
      glVertex2f(subwin.x + end.x - 3,   subwin.y + end.y);                   \
   } glEnd();                                                                 \

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
		dynamic_cast<GLPictureTexture const &>
		(*g_gr->get_road_texture(Widelands::Road_Normal)).get_gl_texture();
	GLuint rt_busy   =
		dynamic_cast<GLPictureTexture const &>
		(*g_gr->get_road_texture(Widelands::Road_Busy)).get_gl_texture();

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


void draw_edges_opengl
	(Rect & subwin,
	 Vertex const & f_vert,
	 Vertex const & r_vert,
	 Vertex const & bl_vert,
	 Vertex const & br_vert,
	 Texture const & tr_d_texture,
	 Texture const &  l_r_texture,
	 Texture const &  f_d_texture,
	 Texture const &  f_r_texture,
	 Texture const & tr_d_edge_texture,
	 Texture const &  f_d_edge_texture,
	 Texture const &  l_r_edge_texture)
{
	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// east edge (horizontal)
	if
		(not (&tr_d_texture == &f_r_texture) // only if underground texture changes
		 and not (f_vert.b == -128 and r_vert.b == -128)) // not at the border to undiscovered land
	{
		glBindTexture(GL_TEXTURE_2D, tr_d_edge_texture.getTexture());
		glBegin(GL_QUADS); {
			{
				GLfloat const brightness = (150.0 + f_vert.b) / 150.0;
				glColor3f(brightness, brightness, brightness);
			}
			glTexCoord2i(0, 0);
			glVertex2f(subwin.x + f_vert.x, subwin.y + f_vert.y);
			{
				GLfloat const brightness = (150.0 + r_vert.b) / 150.0;
				glColor3f(brightness, brightness, brightness);
			}
			glTexCoord2i(TEXTURE_WIDTH, 0);
			glVertex2f(subwin.x + r_vert.x,   subwin.y + r_vert.y);
			glTexCoord2i(TEXTURE_WIDTH, 16);
			glVertex2f(subwin.x + r_vert.x,   subwin.y + r_vert.y + 12);
			{
				GLfloat const brightness = (150.0 + f_vert.b) / 150.0;
				glColor3f(brightness, brightness, brightness);
			}
			glTexCoord2i(0, 16);
			glVertex2f(subwin.x + f_vert.x, subwin.y + f_vert.y + 12);
	   } glEnd();
	}

	// south east edge (vertical)
	if
		(not (&f_d_texture == &f_r_texture)
		 and not (f_vert.b == -128 and br_vert.b == -128))
	{
		glBindTexture(GL_TEXTURE_2D, f_d_edge_texture.getTexture());
		glBegin(GL_QUADS); {
			{
				GLfloat const brightness = (150.0 + f_vert.b) / 150.0;
				glColor3f(brightness, brightness, brightness);
			}
			glTexCoord2i(TEXTURE_WIDTH, 0);
			glVertex2f(subwin.x + f_vert.x, subwin.y + f_vert.y);
			glTexCoord2i(TEXTURE_WIDTH, 16);
			glVertex2f(subwin.x + f_vert.x + 20, subwin.y + f_vert.y);
			{
				GLfloat const brightness = (150.0 + br_vert.b) / 150.0;
				glColor3f(brightness, brightness, brightness);
			}
			glTexCoord2i(0, 16);
			glVertex2f(subwin.x + br_vert.x + 20,   subwin.y + br_vert.y);
			glTexCoord2i(0, 0);
			glVertex2f(subwin.x + br_vert.x,   subwin.y + br_vert.y);
		} glEnd();
	}

	// south west edge (vertical)
	if
		(not (&l_r_texture == &f_d_texture)
		 and not (f_vert.b == -128 and bl_vert.b == -128))
	{
		glBindTexture(GL_TEXTURE_2D, l_r_edge_texture.getTexture());
		glBegin(GL_QUADS); {
			{
				GLfloat const brightness = (150.0 + f_vert.b) / 150.0;
				glColor3f(brightness, brightness, brightness);
			}
			glTexCoord2i(TEXTURE_WIDTH, 0);
			glVertex2f(subwin.x + f_vert.x, subwin.y + f_vert.y);
			glTexCoord2i(TEXTURE_WIDTH, 16);
			glVertex2f(subwin.x + f_vert.x + 20, subwin.y + f_vert.y);
			{
				GLfloat const brightness = (150.0 + bl_vert.b) / 150.0;
				glColor3f(brightness, brightness, brightness);
			}
			glTexCoord2i(0, 16);
			glVertex2f(subwin.x + bl_vert.x + 20,   subwin.y + bl_vert.y);
			glTexCoord2i(0, 0);
			glVertex2f(subwin.x + bl_vert.x,   subwin.y + bl_vert.y);
		} glEnd();
	}
}

#endif
#endif
