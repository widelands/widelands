/*
 * Copyright (C) 2002-2004 by the Wide Lands Development Team
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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>

struct Point
{
   int x;
   int y;

   Point() { }
   Point(int px, int py) : x(px), y(py) { }
};

inline bool operator==(Point a, Point b) { return (a.x == b.x) && (a.y == b.y); }
inline bool operator!=(Point a, Point b) { return (a.x != b.x) || (a.y != b.y); }
// TODO adding points doesn't make sense
inline Point operator+(Point a, Point b) { return Point(a.x + b.x, a.y + b.y); }
inline Point operator-(Point a, Point b) { return Point(a.x - b.x, a.y - b.y); }

struct Rect : public Point {
	int w;
	int h;

	Rect() { }
	Rect(int px, int py, int pw, int ph) : Point(px, py), w(pw), h(ph) { }
};

/** class Point_with_bright
 * this class is like a point, but with additional bright factor
 * bright is an int to make it possible to directly save shifted values (8.8 fixed or so)
 */
struct Point_with_bright : public Point {
   int b;
   Point_with_bright() : Point(0,0) { b=0; }
   Point_with_bright(int px, int py, int pb) : Point(px, py) { b=pb; }
};

/** struct Vertex
 *
 * This replaces Point_with_bright for use with the new texture mapping renderer.
 *
 * This struct is like a point, but with an additional bright factor and texture coordinates.
 */
struct Vertex:public Point {
	int b,tx,ty;
	Vertex (): Point (0,0) { b=tx=ty=0; }
	Vertex (int vx,int vy,int vb,int vtx,int vty): Point (vx,vy)
	{ b=vb; tx=vtx; ty=vty; }
};

// hm, floats...
// tried to be faster with fixed point arithmetics
// it was, but i'll try to find other opts first
class Vector
{
   public:
      float x;
      float y;
      float z;
      Vector()
      {
         x = y = z = 0;
      }
      Vector(float px, float py, float pz)
      {
         x = px; y = py; z = pz;
      }
      void normalize()
      {
         float f = (float)sqrt(x*x + y*y + z*z);
         if (f == 0)
            return;
         x /= f;
         y /= f;
         z /= f;
      }
};

// vector addition
inline Vector operator + (const Vector& a, const Vector& b)
{
   return Vector(a.x + b.x, a.y + b.y, a.z + b.z);
}

// inner product
inline float operator * (const Vector& a, const Vector& b)
{
   return a.x * b.x + a.y * b.y + a.z * b.z;
}


// Structure used to store map coordinates
struct Coords {
   int x;
   int y;

	inline Coords() { }
	inline Coords(int nx, int ny) : x(nx), y(ny) { }
};

inline bool operator==(const Coords& c1, const Coords& c2) { return (c1.x == c2.x) && (c1.y == c2.y); }
inline bool operator!=(const Coords& c1, const Coords& c2) { return !(c1 == c2); }

class Field;

struct FCoords : public Coords {
	Field		*field;
	
	inline FCoords() { }
	inline FCoords(Coords nc, Field *nf) : Coords(nc), field(nf) { }
	inline FCoords(int nx, int ny, Field *nf) : Coords(nx, ny), field(nf) { }
};

#endif /* GEOMETRY_H */
