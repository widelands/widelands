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

#ifndef __S__INPUT_H
#define __S__INPUT_H

#include "singleton.h"
#include "mytypes.h"


typedef int (*MOUSE_CLICK_FUNCTION)(const bool, const uint, const uint, void*);
typedef int (*MOUSE_MOVE_FUNCTION)(const uint, const uint, const signed int, const signed int, const bool, const bool, void*);
typedef int (*KBD_HANDLER)(const char*, void*);


/** class Input 
 *
 * This class cares for mouse and keyboard input
 * You register the functions, that should be called on several input
 * states and the class makes sure that they get checked
 *
 * It's a singleton
 */
#define INPUT_HANDLED 0
#define INPUT_UNHANDLED -1
class Input : public Singleton<Input> {
		  Input(const Input&);
		  Input& operator=(const Input&);

		  public:
					 Input();
					 ~Input();

					 enum	Button {
								BUT1,
								BUT2
					 };

					 void register_mmf(const MOUSE_MOVE_FUNCTION, void* = 0);
					 void register_mcf(const MOUSE_CLICK_FUNCTION, const Button, void* = 0);
					 void register_kbdh(const KBD_HANDLER, void* = 0);
					 void set_mouse_pos(const uint, const uint);
					 void handle_pending_input(void);
					 void grab_input(const bool);
					 void swap_buttons(const bool);
					 void set_mouse_speed(const uint);
					 void set_max_cords(const uint, const uint);

					 /** inline bool Input::should_die(void) 
					  *
					  * This functions indicate if the user requested by user input to exit
					  * (By clicking the Close button in the menu bar e.g.)
					  *
					  * Args: None
					  * Returns: if the app should die
					  */
					 inline bool should_die(void) { return bshould_die; }
					 
					 /** inline uint Input::get_mpx(void) 
					  *
					  * This returns the X mouse pos
					  * 
					  * Args: None
					  * Returns: mouse x cord
					  */
					 inline uint get_mpx(void) { return mpx; }

					 /** inline uint Input::get_mpy(void) 
					  *
					  * This returns the Y mouse pos
					  * 
					  * Args: None
					  * Returns: mouse y cord
					  */
					 inline uint get_mpy(void) { return mpy; }


					 // functions to get the last mouse position
					 inline uint get_mplx(void) { return mplx; }
					 inline uint get_mply(void) { return mply; }

					 /** inline bool Input::is_but_pressed(const Button b) 
					  *
					  * Checks if a button is pressed
					  *
					  * Args: b	button to check
					  * Returns: true if button is pressed
					  */
					 inline bool is_but_pressed(const Button b) { if(b==BUT1) return b1_pressed; else return b2_pressed; }
		 
		  private:
					MOUSE_CLICK_FUNCTION cf[2];
					MOUSE_MOVE_FUNCTION mmf;
					void* cfa[2];
					void* mmfa;
					KBD_HANDLER kbdh;
					void *kbdha;
					uint mpx, mpy, mplx, mply;
					float mouse_speed;
					uint maxx, maxy;
					bool buts_swapped;
					bool input_grabbed;
					bool b1_pressed, b2_pressed;
					bool bshould_die;
					 int levx, levy;

};

#define g_ip	Input::get_singleton()

#endif /* __S__INPUT_H */
