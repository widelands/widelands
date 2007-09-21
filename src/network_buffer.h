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

#include <SDL_net.h>

#include <stdint.h>
#include <string>

/*
 * A Network buffer is a specialisation which knows how
 * to read and write data into a buffer to send it over the
 * network. Data is removed if not requested on read.
 */
struct Network_Buffer {
      Network_Buffer();
      ~Network_Buffer();

      uint16_t size() {return m_buffer_len;}

      uint8_t get_8(bool = true);
      uint16_t get_16(bool = true);
      uint32_t get_32(bool = true);
      std::string get_string(bool = true);

      void put_8(uint8_t);
      void put_16(uint16_t);
      void put_32(uint32_t);
      void put_string(std::string);

      // Return value must not be freed by user
      uint8_t* get_data() {return m_buffer;}

      // Fill this network buffer with all the incoming data
      int fill(TCPsocket);
      void finish();

private:
      void grow_buffer();
      uint32_t m_buffer_real_len;
      uint32_t m_buffer_len;
      uint32_t m_buffer_pointer;
      uint8_t* m_buffer;
};


#endif
