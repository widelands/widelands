// tinygettext - A gettext replacement that works directly on .po files
// Copyright (c) 2006 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_TINYGETTEXT_ICONV_HPP
#define HEADER_TINYGETTEXT_ICONV_HPP

#include <string>

#ifdef TINYGETTEXT_WITH_SDL
#  include "SDL.h"
#else
#  include <iconv.h>
#endif

namespace tinygettext {

namespace detail {
struct ConstPtrHack {
  const char** ptr;
  inline ConstPtrHack(char** ptr_) : ptr(const_cast<const char**>(ptr_)) {}
  inline ConstPtrHack(const char** ptr_) : ptr(ptr_) {}
  inline operator const char**() const { return ptr; }
  inline operator char**() const { return const_cast<char**>(ptr); }
};
} // namespace detail

#ifdef TINYGETTEXT_WITH_SDL
using iconv_t = ::SDL_iconv_t;
#else
using iconv_t = ::iconv_t;
#endif

inline iconv_t iconv_open(const char* tocode, const char* fromcode)
{
#ifdef TINYGETTEXT_WITH_SDL
  return SDL_iconv_open(tocode, fromcode);
#else
  return ::iconv_open(tocode, fromcode);
#endif
}

inline size_t iconv(iconv_t cd,
                    const char** inbuf, size_t* inbytesleft,
                    char** outbuf, size_t* outbytesleft)
{
#ifdef TINYGETTEXT_WITH_SDL
  return SDL_iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
#else
  return ::iconv(cd, detail::ConstPtrHack(inbuf), inbytesleft, outbuf, outbytesleft);
#endif
}

inline int iconv_close(iconv_t cd)
{
#ifdef TINYGETTEXT_WITH_SDL
  return SDL_iconv_close(cd);
#else
  return ::iconv_close(cd);
#endif
}

class IConv
{
private:
  std::string to_charset;
  std::string from_charset;
  iconv_t cd;

public:
  IConv();
  IConv(const std::string& fromcode, const std::string& tocode);
  ~IConv();

  void set_charsets(const std::string& fromcode, const std::string& tocode);
  std::string convert(const std::string& text);

private:
  IConv (const IConv&);
  IConv& operator= (const IConv&);
};

} // namespace tinygettext

#endif

/* EOF */
