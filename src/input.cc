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

#include "widelands.h"

#include <SDL.h>

#include "input.h"

///////////////////////////////////////////////////////////////////////////7

/** class Input 
 *
 * This class cares for mouse and keyboard input
 * You register the functions, that should be called on several input
 * states and the class makes sure that they get checked
 *
 * It's a singleton
 */

/** Input::Input(void) 
 *
 * Default Constructor, simple inits
 *
 * Args: none
 * returns: nothing
 */
Input::Input(void) {
		  cf[0]=0;
		  cf[1]=0;
		  cfa[0]=0;
		  cfa[1]=0;

		  mmf=0;
		  mmfa=0;
		  kbdh=0;
		  kbdha=0;
			
		  mpx=mpy=mplx=mply=0;
		  buts_swapped=0;
		  input_grabbed=0;
		  b1_pressed=b2_pressed=0;
		  mouse_speed=1.0;
		  bshould_die=false;
		  maxx=maxy=0;

		  SDL_ShowCursor(0);
}

/** Input::~Input(void) 
 *
 * Default destructor. Cleans also the arguments given to the functions
 *
 * Agrs: None
 * returns: Nothing
 */
Input::~Input(void) {
/*		  if(cfa[0]) free(cfa[0]);
		  if(cfa[1]) free(cfa[1]);
		  if(mmfa) free(mmfa);
		  if(kbdha) free(kbdha);
		  
		  We do not free this stuff. this makes problems
*/
		  if(input_grabbed) {
					 SDL_WM_GrabInput(SDL_GRAB_OFF);
		  }
		  SDL_ShowCursor(1);
}

/** void Input::set_max_cords(const uint x, const uint y);
 *
 * Sets the maximal screen coordinates to allow as xpos and ypos
 *
 * Args: 	x 	max xpos
 * 			y  max ypos
 * Returns: Nothing
 */
void Input::set_max_cords(uint x, const uint y) {
		  if(maxx==x && maxy==y) return;

		  maxx=x; maxy=y;
		  levx=maxx>>1;
		  levy=maxy>>1;
		  //mplx=mply=levx;
		  //mpy=mply=levy;
		  SDL_WarpMouse(levx, levy);
}

/** void Input::register_mmf(const MOUSE_MOVE_FUNCTION f, void* a= 0)
 *
 * This function registers a mouse move function
 *
 * Args: f	function to register
 * 		a	pointer to userdata
 */
 void Input::register_mmf(const MOUSE_MOVE_FUNCTION f, void* a) {
//			if(mmfa) free(mmfa);

			mmfa=a;
			mmf=f;
 }

/** void Input::register_mcf(const MOUSE_CLICK_FUNCTION f, const Button b, void* a = 0)
 *
 * This function registers a mouse click function
 *
 * Args:	f	function to register
 * 		b 	button to register the function for
 * 		a	pointer to userdata
 */
void Input::register_mcf(const MOUSE_CLICK_FUNCTION f, const Button b, void* a) {
		  uint n;
		  
		  if(b==BUT1) n=0;
		  else n=1;

		  if(buts_swapped) n=!n;
		  
//		  if(cfa[n]) free(cfa[n]);
		  
		  cf[n]=f;
		  cfa[n]=a;
}

/** void Input::register_kbdh(const KBD_HANDLER f, void* a = 0) 
 *
 * This registers the keyboard handler
 *
 * Args:	f 	funcion to register
 * 		a	pointer to userdata
 * Returns: Nothing
 */
void Input::register_kbdh(const KBD_HANDLER f, void* a) {
//		  if(kbdha) free(kbdha);

		  kbdh=f;
		  kbdha=a;
}

/** void Input::swap_buttons(const bool b) 
 *
 * This functions tells if buttons should be swapped or not.
 * Note: if buttons are swapped is checked when functions get registered
 * 		not when there's actually a click
 *
 * Args: b 	tells if swapping is to be turned on or off
 * Returns: nothing
 */
void Input::swap_buttons(const bool b) { buts_swapped=b; }
	
/** void Input::grab_input(const bool b)
 *
 * should mouse cursor and keyboard input be grabbed?
 *
 * Args: b 	grab on/off
 * Returns: Nothing
 */
void Input::grab_input(const bool b) {
		  input_grabbed=b;

		  if(input_grabbed) {
					 SDL_WM_GrabInput(SDL_GRAB_ON);
		  } else {
					 SDL_WM_GrabInput(SDL_GRAB_OFF);
		  }
}

/** void Input::set_mouse_speed(const uint s) 
 *
 * Sets the mouse speed in per cent
 *
 * Args:	s	mousespeed in per cent
 * Returns: Nothing
 */
void Input::set_mouse_speed(const uint s)  { if(!s) return; mouse_speed=((float) s/100); }

/** void Input::set_mouse_pos(uint x, uint y)
 *
 * Sets the mouse pos
 *
 * Args:	x	xpos of mouse
 * 		y	ypos of mouse
 * Returns: Nothing
 */
void Input::set_mouse_pos(uint x, uint y) { mpx=mplx=x; mpy=mply=y; }

/** void Input::handle_pending_input(void)
 *
 * This function is the heart of the whole class
 * it should be called frequently (read: as often as possible)
 * and cares about the registered input events and the update
 * of the status information
 *
 * Args: None
 * Returns: Nothing
 */
void Input::handle_pending_input(void) {
		  static SDL_Event ev;
		  static int xdiff, ydiff;
		  SDL_PumpEvents();

		  while(SDL_PollEvent(&ev)) {
					 switch(ev.type) {
								case SDL_KEYDOWN:
										  if (ev.key.keysym.sym == SDLK_F10) // TEMP
										  		bshould_die = true;
										  if(kbdh) kbdh(SDL_GetKeyName(ev.key.keysym.sym), kbdha);
										  break;

								case SDL_KEYUP:
										  //                              DBG("Key released!\n");
										  break;


								case SDL_MOUSEBUTTONDOWN:
								case SDL_MOUSEBUTTONUP:
									uint but;
									if(ev.button.button==3) but=1;
									else if(ev.button.button==1) but=0;
									else break;

									if(cf[but])
										cf[but](ev.button.state ? true : false, mpx, mpy, cfa[but]);
									break;

								case SDL_MOUSEMOTION:
									// This seems to produce inconsistent values for xdiff/ydiff
									if(ev.motion.x!=maxx>>1 || ev.motion.y!=maxy>>1) {
										mplx=mpx; mply=mpy;
										xdiff = (int) ((ev.motion.x-levx)*mouse_speed);
										ydiff = (int) ((ev.motion.y-levy)*mouse_speed);
										mpx+=xdiff;
										mpy+=ydiff;

										if((int)mpx < 0) mpx=0;
										else if(mpx >=maxx) mpx=maxx-1;
										if((int)mpy < 0) mpy=0;
										else if(mpy >=maxy) mpy=maxy-1;

										if(mmf) {
											if(!buts_swapped) {
												mmf(mpx, mpy, xdiff, ydiff, (SDL_BUTTON(1) & ev.motion.state) ? true : false,
														(SDL_BUTTON(3) & ev.motion.state) ? true : false, mmfa);
											} else {
												mmf(mpx, mpy, xdiff, ydiff, (SDL_BUTTON(3) & ev.motion.state) ? true : false,
														(SDL_BUTTON(1) & ev.motion.state) ? true : false, mmfa);
											}
										}

										levx=ev.motion.x;
										levy=ev.motion.y;

										SDL_WarpMouse(maxx>>1,maxy>>1);
									} else {
										levy=maxy>>1;
										levx=maxx>>1;
									}
									break;

								case SDL_QUIT:
										  bshould_die=true;
										  break;

								default:
										  break;
					 }
		  }
}

