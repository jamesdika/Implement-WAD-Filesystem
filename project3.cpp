// project3.cpp : Defines the entry point for the console application.

#include <iostream>
#include <string>
#include "wad.h"
#include <fstream>
#include <cstring>

// TODO: Fix the spot where you check if file has 'E#M#' because right now its janky


	std::string static trim(std::string s){
		std::string trimmed = "";
		for(int i = 0; i < s.length(); i++){
			if(isprint(s[i]) != 0){
				trimmed += s[i];
			}
		}
		return trimmed;
}


	std::string static getData(uint32_t offset, uint32_t length, const std::string &path) {
	std::string contents = ""; //contents of file
	std::ifstream file;
	file.open(path);
	char temp;
	file.seekg(offset); // go to start of file
	for (int i = 0; i < length; i++) {
		file.get(temp);
		contents += temp;
	}
	file.close();
	return contents;
}

int Wad::getIndex(const std::string &path) {
	int depth = 1;
	if(path[0] == '/' and path.length() == 1){ //check if its only looking for root
		return -2;
	}

	std::string data = path; // since path is constant i had to recopy it into a new string

	if(data[data.length() - 1] == '/'){ // if last character is \, i remove it
		data = data.substr(0, data.length() - 1);
	}

	// if the first character of path is root directory and its not the only character, remove it
	// I know I'm looking for another directory anyway
	if(data[0] == '/' and data.length() != 1){
		data = trim(data.substr(1)); // might have a loophole here, whenever i do substring string has some extra stuff
	}


	for(int i = 0; i < this->files.size(); i++){
		//check if there is no '\\' left, if there isnt then youre working with last file of the path, enter this block
		if(data.find('/') == std::string::npos){
			if(data.compare(this->files[i].name) == 0 and depth == this->files[i].depth){ // found the match, depth matches
				return i;
				}
			else{
				continue;
			}
		}

		// if you find the file thats equal, then remove part of string up to and including next backslash
		if(data.substr(0, data.find_first_of('/')).compare(this->files[i].name) == 0 and depth == this->files[i].depth){
			data = trim(data.substr(data.find_first_of('/') + 1));
			depth++;
			continue;
		}
	}
	return -1;
}

Wad* Wad::loadWad(const std::string &path) { //object allocator- creates WAD object an loads WAD file data from path into memory
	// first parse for magic, number of descriptors, descriptor offset
	// then go to the descriptor table
	// create a file struct for each descriptor and push it to the vector
	std::ifstream file;
	file.open(path);
	if (!file.is_open()) {
		std::cout << "Wrong path entered" << std::endl;
		return nullptr;
	}

	Wad* wad = new Wad(); // wad pointer to be returned
	uint32_t num_descriptors = 0;
	uint32_t descriptor_offset = 0;
	char magic[5];
	// get magic
	for (int i = 0; i < 4; i++) {
		file.get(magic[i]);
	}
	wad->magic = magic;
	// get number of descriptors
	for (int i = 0; i < 4; i++) {
		char temp;
		file.get(temp);
		num_descriptors += (unsigned char)(temp) << 8 * i;
	}
	// get offset
	for (int i = 0; i < 4; i++) {
		char temp;
		file.get(temp);
		descriptor_offset += (unsigned char)(temp) << 8 * i;
	}
	
	// move to location of table of descriptors
	file.seekg(descriptor_offset);
	int depth = 1; // set directory level to 1 
	int map_marker_count = 0; // set mapMarker = 0

	// main loop to go through descriptors
	for (int i = 0; i < num_descriptors; i++) {

		uint32_t offset = 0;
		uint32_t length = 0;
		std::string name = "";
		char temp;

		// get offset- 4
		for (int j = 0; j < 4; j++) {
			file.get(temp);
			offset += (unsigned char)(temp) << 8 * j;
		}

		// then get length- 4 
		for (int j = 0; j < 4; j++) {
			file.get(temp);
			length += (unsigned char)(temp) << 8 * j;
		}

		// then get name- 8
		for (int j = 0; j < 8; j++) {
			file.get(temp);
			name += temp;
		}




		// if the length is 0 and has start, append, then increment depth


		if (length == 0 && (name.find("_START") != std::string::npos) ) {
			finfo file = { depth, length, offset, trim(name.substr(0, name.find('_'))), getData(offset, length, path) }; // WARNING careful here
			wad->files.push_back(file);
			depth++;
			continue;
		}


		// if the length is 0 and has end then decrement depth
		else if (length == 0 && (name.find("_END") != std::string::npos)) {
			depth--;
			continue;
		}


		// if length is 0 and em is present, append to vector, increment depth and make note of it by setting map_marker_count to 10
		else if (length == 0 && (name.find('E') != std::string::npos) && (name.find('M') != std::string::npos)) {
			finfo file = { depth, length, offset, trim(name), getData(offset, length, path) };
			wad->files.push_back(file);
			depth++;
			map_marker_count = 10;
			continue;
		}


		// if map_marker_down > 0, then append to vector and decrement map_marker_count by 1. if it becomes 0, decrement depth by 1
		else if (map_marker_count > 0) {
			finfo file = { depth, length, offset, trim(name), getData(offset, length, path) };
			wad->files.push_back(file);
			map_marker_count--;
			if (map_marker_count == 0) {
				depth--;
			}
			continue;
		}


		else {
			finfo file = { depth, length, offset, trim(name), getData(offset, length, path) };
			wad->files.push_back(file);
		}
	}
	return wad;
}

char* Wad::getMagic() { // returns magic for this wad data
	return this->magic;
}


bool Wad::isContent(const std::string &path) { // returns true if path represents content(data), and false otherwise
	int index = this->getIndex(path);
	if(index == -2){
		return false;
	}
	if(index == -1){
		std::cout << "Invalid path" << std::endl;
		return false;
	}
	if(this->files[index].length != 0){
		return true;
	}
	else{
		return false;
	}
}


bool Wad::isDirectory(const std::string &path) { // if path represents directory, and false otherwise
	int index = this->getIndex(path);
	if(index == -2){
		return true;
	}
	if(index == -1){
		std::cout<< "Invalid path" << std::endl;
		return false;
	}
	if(this->files[index].length == 0){ // if it has length 0 its a directory
		return true;
	}
	else{
		return false; // otherwise its not
	}
}

int Wad::getSize(const std::string &path) { // if path represents content, return the number of bytes in its data; otherwise, returns -1
	int index = this->getIndex(path);
	if(this->isContent(path)){ // if it has length 0 its a directory
		return this->files[index].length;
	}
	else{
		return -1; // otherwise its not
	}
}


/*
 * If path  represents  content,  copies  up  to  length  bytes  of  its  data  into  buffer.
 * If offset  is  provided,  data  should be copied starting from that byte in the content.
 * Returns the number of bytes copied into the buffer, or -1 if path does not represent
 * content (e.g., if it represents a directory).
 */
int Wad::getContents(const std::string &path, char *buffer, int length, int offset) {
	int index = this->getIndex(path);
	int count = offset;
	int j = 0;
	if(this->isContent(path)){
		while(count < length){
			buffer[j] = this->files[index].contents[count]; // contents equal to every byte starting from offset
			count++;
			j++;
		}
		return (length - offset);
	}
	else{
		return -1;
	}

}


/*
 * If path  represents a directory, places entries for contained elements in directory.
 * Returns  the  number  of  elements in the directory, or -1 if path does not represent
 * a directory (e.g., if it represents content).
 */
int Wad::getDirectory(const std::string &path, std::vector<std::string> *directory) {
	int targetDepth; // depth of contents
	int index = this->getIndex(path); // index of path
	int tracker; // index where i start collecting contents of path
	int count = 0;

	if(index == -1){
		std::cout<<"Invalid path" << std::endl;
		return -1;
	}
	// if they give you the root '/', then just go through entire loop and push all files with depth = 1
	else if(index == -2){
		for(int i = 0; i < this->files.size(); i++){
			if(this->files[i].depth == 1){
				directory->push_back(this->files[i].name);
				count++;
			}
		}
		return count;
	}
	else {
		//if its contents
		if(this->files[index].length != 0){
			return -1;
		}
		else{
			targetDepth = this->files[index].depth + 1;
			tracker = index + 1;
			while(this->files[index].depth != this->files[tracker].depth and tracker != this->files.size()){
				if(this->files[tracker].depth == targetDepth){
					directory->push_back(this->files[tracker].name);
					count++;
				}
				tracker++;
			}
			return count;
		}
	}
}


void exploreDirectory(Wad *data, const std::string path, int level)
{
	for (int index = 0; index < level; index++)
		std::cout << " ";

	std::vector<std::string> entries;
	std::cout << "[Objects at this level:" << data->getDirectory(path, &entries) << "]" << std::endl;

	for (std::string entry : entries)
	{
		std::string entryPath = path + entry;

		for (int index = 0; index < level; index++)
			std::cout << " ";

		if (data->isDirectory(entryPath))
		{
			std::cout << level << ". DIR: " << entry << std::endl;
			exploreDirectory(data, entryPath + "/", level + 1);
		}
		else if (data->isContent(entryPath))
			std::cout << level << ". CONTENT: " << entry << "; Size: " << data->getSize(entryPath) << std::endl;
		else
			std::cout << "***WARNING: entry " << entry << " has invalid type!***" << std::endl;
	}
}

void exploreDirectory(Wad *data, const std::string path)
{
	std::cout << "EXPLORING: " << path << std::endl;
	exploreDirectory(data, path, 1);
}

int main(int argc, char** argv)
{
	Wad *myWad = Wad::loadWad("C:\\Users\\james\\Downloads\\sample.wad");
	exploreDirectory(myWad, "/");
	delete myWad;
}