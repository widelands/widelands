/* WordFile format 0.1

2002-02-10	sft+	created

WordFile
{
	WordFileHeader	h;
	Picture			p[h.nPictures];
	TerrainType		t[h.nTerrains];
	Bob				b[h.nBobs];
	ResourceDesc	r[h.nResources];
}
*/

typedef unsigned short word;
typedef unsigned int dword;

#define WLWF_MAGIC		"WLwf"
#define WLWF_VERSION	0x0001
#define WLWF_VERSIONMAJOR(a)	(a >> 8)
#define WLWF_VERSIONMINOR(a)	(a & 0xFF)

struct WorldFileHeader
{
	char magic[6];			// "WLwf\0\0"
	word version;			// file format version
	char author[20];
	char name[20];
	dword checksum;			// not yet used
	dword nPictures;
	dword nTerrains;
	dword nBobs;
	dword nResources;
};

struct TerrainType
{
	char name[16];
	dword attributes;
	dword texture;			// index in WorldFileHeader::picture
};

/*struct Bob
{
	char name[16];
	dword atributes;
	dword texture;
	//?
};*/
typedef struct TerrainType Bob;		// for now, that'll do just fine

struct ResourceDesc
{
	char name[16];
	dword atributes;
	dword texture;
	dword occurence;		// how much of this is will be in this world
							// (we'll have to define some unit for this)
	//?
};

enum PictureType
{
	PIC_OTHER,
	PIC_TERRAIN,
	PIC_BOB,
	PIC_RESOURCE,
	FORCE_DWORD = 0x7FFFFFF		// trying to force this enum to compile
								// to same size on different compilers;
								// we'll see if it works
};

struct Picture
{
	// name & type: not really needed, but safe, editor- & debugger-friendly
	char name[16];
	PictureType type;		// terrain-texture/bob-picture/resource-picture
	
	dword width;
	dword heigth;
	dword depth;			// bits per pixel
	dword flags;
	// image data
};