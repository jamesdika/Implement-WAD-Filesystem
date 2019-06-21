//
// Created by james on 3/29/2019.
//
#include <iostream>
#include <string>
#include "wad.h"
#include <vector>
#include <fuse.h>
#include <stdio.h>

static Wad* wad;

int getattr_callback(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    if (wad->isDirectory(path)) {
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
        return 0;
    }

    if (wad->isContent(path)) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = wad->getSize(path);
        return 0;
    }

    return -ENOENT;
}

int read_callback(const char *path, char *buf, size_t size, off_t offset,
                         struct fuse_file_info *fi) {

    if (wad->isContent(path)) {
        if(offset > size){
            return 0;
        }
        return wad->getContents(path, buf, size, offset);
    }

    return -ENOENT;
}

int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
                            off_t offset, struct fuse_file_info *fi) {

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    std::vector<std::string> contents; // contents of directory
    if(wad->isDirectory(path)){
        wad->getDirectory(path, &contents);
        for (std::string content : contents){
            filler(buf, content, NULL, 0);
        }
        return 0;
    }
    else{
        return ENOENT;
    }
}

int open_callback(const char * path, struct fuse_file_info *){
    if(wad->isContent(path)){
        return 0;
    }
    else{
        return ENOENT;
    }
}

int opendir_callback(const char * path, struct fuse_file_info *){
    if(wad->isDirectory(path)){
        return 0;
    }
    else{
        return ENOENT;
    }
}

int release_callback(const char * path, struct fuse_file_info *){
    if(wad->isContent(path)){
        return 0;
    }
    else{
        return -1;
    }
}

int releasedir_callback(const char * path, struct fuse_file_info *){
    if(wad->isDirectory(path)){
        return 0;
    }
    else{
        return -1;
    }
}
static struct fuse_operations fuse_obj;

int main(int argc, char** argv){
    if(argc < 2){
        std::cout << "Enter your filesystem path as a command line argument."
        exit(EXIT_SUCCESS);
    }
    std::string path(argv[1]);
    wad = Wad::loadWad(path);
    fuse_obj.getattr = getattr_callback,
    fuse_obj.open = open_callback,
    fuse_obj.opendir = opendir_callback,
    fuse_obj.read = read_callback,
    fuse_obj.readdir = readdir_callback,
    fuse_obj.release = release_callback,
    fuse_obj.releasedir = releasedir_callback,
    int retVal = fuse_main(argc, arguments, &fuse_obj, NULL);
    delete wad;
    return retVal;
}
