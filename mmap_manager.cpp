#include <sys/mman.h>//mmap, munmap
#include <sys/stat.h>//open
#include <fcntl.h>//O_LARGEFILE, O_RDWR, O_RDONLY
#include <unistd.h> //close
#include <cstdlib>

#include <iostream>
#include <fstream>
#include "mmap_manager.h"

namespace{

size_t get_filesize(const char *fn){
    std::ifstream ifs(fn);
    if(!ifs){
        std::cerr << "Failed to open " << fn << std::endl;
        std::abort();
    }
    return ifs.seekg(0, std::ios::end).tellg();
}

}

struct mmap_manager::impl{
    int fd;
    void *mapped_area;
    size_t mapped_size;
    impl(const char *, bool, size_t);
    ~impl();
};

mmap_manager::impl::impl(const char *filename, bool is_writable, size_t size){
    fd = open(filename, O_LARGEFILE | O_SYNC | (is_writable ? O_RDWR : O_RDONLY));
    if(fd < 0){
        perror(filename);
        std::abort();
    }
    mapped_size = size;
    mapped_area = mmap(NULL, size, PROT_READ | (is_writable ? PROT_WRITE : 0), MAP_SHARED, fd, 0);
    if(mapped_area == MAP_FAILED){
        perror(filename);
        std::abort();
    }
}

mmap_manager::impl::~impl(){
    if(msync(mapped_area, mapped_size, MS_SYNC)){
        perror("msync");
        std::abort();
    }
    if(munmap(mapped_area, mapped_size)){
        perror("munmap");
        std::abort();
    }
    if(close(fd)){
        perror("close");
        std::abort();
    }
}


mmap_manager::mmap_manager(const char *filename, bool is_writable){
    pimpl = new impl(filename, is_writable, get_filesize(filename));
}

mmap_manager::mmap_manager(const char *filename, bool is_writable, size_t map_size){
    pimpl = new impl(filename, is_writable, std::min(get_filesize(filename), map_size));
}

mmap_manager::~mmap_manager(){
    delete pimpl;
}

void * mmap_manager::get_ptr()const{
    return pimpl->mapped_area;
}

size_t mmap_manager::get_size()const{
    return pimpl->mapped_size;
}
