#include "song.h"

Song::Song(std::string& file)
{
    filename = file;
    title = file;
    enabled = true;
}

