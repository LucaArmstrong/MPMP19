#ifndef FILE_HANDLE_HPP
#define FILE_HANDLE_HPP

#include <cstdio>
#include <string>
#include "mpmp19_error.hpp"

namespace mpmp19 {

struct FileHandle {
    FILE* ptr = nullptr;
    explicit FileHandle(const char* path, const char* mode) {
        ptr = fopen(path, mode);
        if (!ptr) 
            throw mpmp19_error("Error opening file named " + std::string(path));
    }
    ~FileHandle() { if (ptr) fclose(ptr); }
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
};

}   // namespace

#endif