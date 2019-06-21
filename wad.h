#pragma once

#include <vector>
#include <string>

class Wad{
	struct finfo {
		int depth; // level, if depth is 1 - root, if 2 - has 1 parent
		uint32_t length; // size of file
		uint32_t offset; // where its located
		std::string name; // string name
		std::string contents; // string contents
	};

	public:
		std::vector<finfo> files;
		char* magic; //changed from string

		static Wad* loadWad(const std:: string &path); //functions as constructor

		char* getMagic();

		bool isContent(const std::string &path);

		bool isDirectory(const std::string &path);

		int getSize(const std::string &path);

		int getContents(const std::string &path, char *buffer, int length, int offset = 0);

		int getDirectory(const std::string &path, std::vector<std::string> *directory);

		int getIndex(const std::string &path);

};