#include "../src/mytypes.h"
#include "../src/worldfiletypes.h"

class Growable_Array;
class Profile;

class Conf_Reader
{
	Profile*				conf;
	WorldFileHeader			header;
	ResourceDesc*			resource;
	TerrainType*			terrain;
	Growable_Array*			pics;
	BobDesc*				bob;
	void					read_header();
	void					read_resources();
	void					read_terrains();
	void					read_bobs();
public:
							Conf_Reader(const char* filename);
							~Conf_Reader();
	const WorldFileHeader*	get_header();
	const ResourceDesc*		get_resource(int n);
	const TerrainType* 		get_terrain(int n);
	const BobDesc*	 		get_bob(int n);
	uint					textures();
	const char*				get_texture(int n);
};