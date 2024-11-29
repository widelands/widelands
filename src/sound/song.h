#ifndef SONG_H
#define SONG_H

#include <string>

struct Song
{
public:
    Song(std::string& filename);
    std::string filename;
    std::string title;
    bool enabled;
};

#endif // SONG_H
