#include "../src/mytypes.h"

struct PictureInfo;
class Binary_file;

class Picture_Reader
{
	char dir[1024];
	void read_bmp(Binary_file* file, PictureInfo* pic, ushort** data);
public:
	Picture_Reader(const char* picdir);
	~Picture_Reader();
	void read_picture(const char* name, PictureInfo* pic, ushort** data);
};