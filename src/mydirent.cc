#ifdef WIN32
#include "dirent.h"

Dir::Dir(const char* n)
{
	name = new char[strlen(n) + 1 + 4];
	strcpy(name, n);
	strcat(name, "\\*.*");
	find = NULL;
}

Dir::~Dir()
{
	if (find)
		FindClose(find);
	delete name;
}

DIR* opendir(const char* name)
{
	return new DIR(name);
}

dirent* readdir(DIR* dir)
{
	WIN32_FIND_DATA wfd;
	if (!dir->find)
	{
		dir->find = FindFirstFile(dir->name, &wfd);
		if (dir->find == INVALID_HANDLE_VALUE)
			return NULL;
	}
	else
		if (!FindNextFile(dir->find, &wfd))
			return NULL;

	while (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		if (!FindNextFile(dir->find, &wfd))
			return NULL;

	strcpy(dir->ret.d_name, wfd.cFileName);
	return &dir->ret;
}

void closedir(DIR* dir)
{
	delete dir;
}

#endif //WIN32