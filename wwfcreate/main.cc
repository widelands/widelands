#include <stdio.h>
#include <string.h>
//#include "../src/mydirent.h"
#include "../src/worldfiletypes.h"
#include "../src/myfile.h"
#include "confreader.h"

#define CONF_NAME		"conf"
#define PICTURE_DIR		"pics"
#define WWF_EXT			".wwf"
#ifdef WIN32
#define SEPERATOR		'\\'
#else
#define SEPERATOR		'/'
#endif

#define VERSION			"0.9pre"

char dirname[1024];
char outfile[1024];

void printHelp()
{
	printf("creates wide lands world files.\n");
	printf("usage:\n");
	printf("\twwfcreate <inputdir> [outputfile]\n");
}

void wwfcreate(Binary_file* output)
{
	char confName[1024];
	strcpy(confName, dirname);
	strcat(confName, CONF_NAME);
	Conf_Reader* conf = new Conf_Reader(confName);
	
	// first, the header.
	const WorldFileHeader* head = conf->get_header();
	output->write(head, sizeof(WorldFileHeader));

	for (uint i=0; i<head->resources; i++)
		output->write(conf->get_resource(i), sizeof(ResourceDesc));

	for (uint j=0; j<head->terrains; j++)
		output->write(conf->get_terrain(j), sizeof(TerrainType));

	for (uint k=0; k<head->bobs; k++)
		output->write(conf->get_bob(k), sizeof(BobDesc));
	
	for (uint m=0; m<conf->textures(); m++)
	{
		const char* picName = conf->get_texture(m);
		// read + write pics
		printf("picture writing not implemented:\t#%i\t(%s)\n", m, picName);
	}
	delete conf;
}

int main(int argc, char* argv[])
{
	printf("Wide Lands WorldMaker v%s\n", VERSION);
	if (argc == 1 || argc > 3)
		printHelp();

	// get input dir name
	strcpy(dirname, argv[1]);
	if (dirname[strlen(dirname) - 1] != SEPERATOR)
	{
		dirname[strlen(dirname)] = SEPERATOR;
		dirname[strlen(dirname) + 1] = 0;
	}
	// get output file name
	if (argc == 3)
		strcpy(outfile, argv[2]);
	else
	{
		strcpy(outfile, dirname);
		outfile[strlen(outfile)-1] = 0;
		strcat(outfile, WWF_EXT);
	}
	
	Binary_file* output = new Binary_file();
	output->open(outfile, File::For::WRITE);
	wwfcreate(output);
	delete output;

	getchar();
	return 0;
}