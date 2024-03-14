// tinygettext - A gettext replacement that works directly on .po files
// Copyright (c) 2009 Ingo Ruhnke <grumbel@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "tinygettext/po_parser.hpp"
#include "tinygettext/tinygettext.hpp"
#include "tinygettext/log.hpp"

namespace {

void my_log_callback(const std::string& err)
{
  std::cerr << err;
}

} // namespace

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout << argv[0] << " FILENAME..." << std::endl;
  }
  else
  {
    tinygettext::Log::set_log_info_callback(my_log_callback);
    tinygettext::Log::set_log_warning_callback(my_log_callback);
    tinygettext::Log::set_log_error_callback(my_log_callback);

    for(int i = 1; i < argc; ++i)
    {
      std::ifstream in(argv[i]);
      if (!in)
      {
        std::cerr << argv[0] << ": cannot access " << argv[i] << ": " << strerror(errno) << std::endl;
      }
      else
      {
        try
        {
          tinygettext::Dictionary dict1;
          tinygettext::POParser::parse(argv[i], in, dict1);

          //tinygettext::Dictionary dict2;
          //tinygettext::POFileReader::read(in, dict2);
        }
        catch(std::runtime_error& err)
        {
          std::cout << argv[i] << ": exception: " << err.what() << std::endl;
        }
      }
    }
  }
}

/* EOF */
