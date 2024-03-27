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

#include <iostream>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include "tinygettext/po_parser.hpp"
#include "tinygettext/tinygettext.hpp"
#include "tinygettext/unix_file_system.hpp"

using namespace tinygettext;

namespace {

void print_msg(const std::string& msgid, const std::vector<std::string>& msgstrs)
{
  std::cout << "Msgid: " << msgid << std::endl;
  for(std::vector<std::string>::const_iterator i = msgstrs.begin(); i != msgstrs.end(); ++i)
    {
      std::cout << *i << std::endl;
    }
}

void print_msg_ctxt(const std::string& ctxt, const std::string& msgid, const std::vector<std::string>& msgstrs)
{
  std::cout << "Msgctxt: " << ctxt << std::endl;
  std::cout << "Msgid: " << msgid << std::endl;
  for(std::vector<std::string>::const_iterator i = msgstrs.begin(); i != msgstrs.end(); ++i)
    {
      std::cout << *i << std::endl;
    }
}

void print_usage(int /*argc*/, char** argv)
{
  std::cout << "Usage: " << argv[0] << " translate FILE MESSAGE" << std::endl;
  std::cout << "       " << argv[0] << " translate FILE MESSAGE_S MESSAGE_P NUM" << std::endl;
  std::cout << "       " << argv[0] << " directory DIRECTORY MESSAGE [LANG]" << std::endl;
  std::cout << "       " << argv[0] << " language LANGUAGE" << std::endl;
  std::cout << "       " << argv[0] << " language-dir DIR" << std::endl;
  std::cout << "       " << argv[0] << " list-msgstrs FILE" << std::endl;
}

void read_dictionary(const std::string& filename, Dictionary& dict)
{
  std::ifstream in(filename.c_str());

  if (!in)
    {
      throw std::runtime_error("Couldn't open " + filename);
    }
  else
    {
      POParser::parse(filename, in, dict);
      in.close();
    }
}

} // namespace

int main(int argc, char** argv)
{
  try
  {
    if (argc == 3 && strcmp(argv[1], "language-dir") == 0)
    {
      DictionaryManager dictionary_manager(std::unique_ptr<tinygettext::FileSystem>(new UnixFileSystem));
      dictionary_manager.add_directory(argv[2]);
      const std::set<Language>& languages = dictionary_manager.get_languages();
      std::cout << "Number of languages: " << languages.size() << std::endl;
      for (std::set<Language>::const_iterator i = languages.begin(); i != languages.end(); ++i)
      {
        const Language& language = *i;
        std::cout << "Env:       " << language.str()           << std::endl
                  << "Name:      " << language.get_name()      << std::endl
                  << "Language:  " << language.get_language()  << std::endl
                  << "Country:   " << language.get_country()   << std::endl
                  << "Modifier:  " << language.get_modifier()  << std::endl
                  << std::endl;
      }
    }
    else if (argc == 3 && strcmp(argv[1], "language") == 0)
    {
      const char* language_cstr = argv[2];
      Language language = Language::from_name(language_cstr);

      if (language)
        std::cout << "Env:       " << language.str()           << std::endl
                  << "Name:      " << language.get_name()      << std::endl
                  << "Language:  " << language.get_language()  << std::endl
                  << "Country:   " << language.get_country()   << std::endl
                  << "Modifier:  " << language.get_modifier()  << std::endl;
      else
        std::cout << "not found" << std::endl;
    }
    else if (argc == 4 && strcmp(argv[1], "translate") == 0)
    {
      const char* filename = argv[2];
      const char* message  = argv[3];

      Dictionary dict;
      read_dictionary(filename, dict);
      std::cout << "TRANSLATION: \"\"\"" << dict.translate(message) << "\"\"\""<< std::endl;
    }
    else if (argc == 5 && strcmp(argv[1], "translate") == 0)
    {
      const char* filename = argv[2];
      const char* context  = argv[3];
      const char* message  = argv[4];

      Dictionary dict;
      read_dictionary(filename, dict);
      std::cout << dict.translate_ctxt(context, message) << std::endl;
    }
    else if (argc == 6 && strcmp(argv[1], "translate") == 0)
    {
      const char* filename = argv[2];
      const char* message_singular = argv[3];
      const char* message_plural   = argv[4];
      int num = atoi(argv[5]);

      Dictionary dict;
      read_dictionary(filename, dict);
      std::cout << dict.translate_plural(message_singular, message_plural, num) << std::endl;
    }
    else if (argc == 7 && strcmp(argv[1], "translate") == 0)
    {
      const char* filename = argv[2];
      const char* context  = argv[3];
      const char* message_singular = argv[4];
      const char* message_plural   = argv[5];
      int num = atoi(argv[6]);

      Dictionary dict;
      read_dictionary(filename, dict);
      std::cout << dict.translate_ctxt_plural(context, message_singular, message_plural, num) << std::endl;
    }
    else if ((argc == 4 || argc == 5) && strcmp(argv[1], "directory") == 0)
    {
      const char* directory = argv[2];
      const char* message   = argv[3];
      const char* language  = (argc == 5) ? argv[4] : nullptr;

      DictionaryManager manager(std::unique_ptr<tinygettext::FileSystem>(new UnixFileSystem));
      manager.add_directory(directory);

      if (language)
      {
        Language lang = Language::from_name(language);
        if (lang)
        {
          manager.set_language(lang);
        }
        else
        {
          std::cout << "Unknown language: " << language << std::endl;
          exit(EXIT_FAILURE);
        }
      }

      std::cout << "Directory:   '" << directory << "'"  << std::endl;
      std::cout << "Message:     '" << message << "'" << std::endl;
      std::cout << "Language:    '" << manager.get_language().str() << "' (name: '"
                << manager.get_language().get_name() << "', language: '"
                << manager.get_language().get_language() << "', country: '"
                << manager.get_language().get_country() << "', modifier: '"
                << manager.get_language().get_modifier() << "')"
                << std::endl;
      std::cout << "Translation: '" << manager.get_dictionary().translate(message) << "'" << std::endl;
    }
    else if ((argc == 3) && strcmp(argv[1], "list-msgstrs") == 0)
    {
      const char* filename = argv[2];

      Dictionary dict;
      read_dictionary(filename, dict);
      dict.foreach(print_msg);
      dict.foreach_ctxt(print_msg_ctxt);
    }
    else
    {
      print_usage(argc, argv);
    }
  }
  catch(std::exception& err)
  {
    std::cout << "Exception: " << err.what() << std::endl;
  }

  return 0;
}

/* EOF */
