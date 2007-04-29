/*
 * Copyright (C) 2002-2004, 2007 by the Widelands Development Team
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

#ifndef __S__NETWORK_BUFFER_H
#define __S__NETWORK_BUFFER_H

#include "types.h"

#include <SDL_net.h>

#include <string>

/*
 * A Network buffer is a specialisation which knows how
 * to read and write data into a buffer to send it over the
 * network. Data is removed if not requested on read.
 */
struct Network_Buffer {
      Network_Buffer(void);
      ~Network_Buffer(void);

      ushort size(void) { return m_buffer_len; }

      uchar get_8(bool = true);
      ushort get_16(bool = true);
      uint get_32(bool = true);
      std::string get_string(bool = true);

      void put_8(uchar);
      void put_16(ushort);
      void put_32(uint);
      void put_string( std::string );

      // Return value must not be freed by user
      uchar* get_data() { return m_buffer; }

      // Fill this network buffer with all the incoming data
      int fill(TCPsocket);
      void finish( void );

private:
      void grow_buffer();
      uint m_buffer_real_len;
      uint m_buffer_len;
      uint m_buffer_pointer;
      uchar* m_buffer;
};


#endif
