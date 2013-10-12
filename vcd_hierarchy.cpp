#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>

#include "mmap_manager.h"
#include "vcd_header.h"

namespace{

//! count the size of VCD header in Byte
//
//! @param filename VCD file
//! @return size of VCD header
size_t get_vcd_header_size(const char *filename){
    std::ifstream ifs(filename);
    if(!ifs){
        std::cerr << "Failed to open " << filename << std::endl;
        std::abort();
    }
    size_t size = 0;
    for(std::string line; ifs && std::getline(ifs, line); ){
        if(line == "$dumpvars"){
            assert(line.size() == 9);
            return size;
        }
        size += line.size() + 1; //+1 is a sizeof('\n')
    }
    assert(!"Failed to read header");
}

//! RAII idiom for File descriptor
struct fp_raii{
    std::FILE *const fp;
    explicit fp_raii(FILE *f) : fp(f){}
    ~fp_raii(){if(fp) std::fclose(fp);}
    operator std::FILE *()const{return fp;}
};

//! Open File in create mode and write the whole data
//
//! @param orig_vcd Original VCD filename
//! @param output_file New VCD file
//! @param v header information
//! @param header_size size of vcd header
int make_new_file_and_write(const char *orig_vcd, const char *output_file, const std::vector<char> &v, size_t header_size){
    fp_raii ofp(std::fopen(output_file, "w"));
    if(!ofp){
        perror(output_file);
        return -1;
    }
    fp_raii ifp(std::fopen(orig_vcd, "r"));
    if(!ifp){
        perror(orig_vcd);
        return -1;
    }
    for(size_t written = 0; written < v.size(); ){
        written += std::fwrite(&v.front() + written, 1, v.size() - written, ofp);
    }
    std::vector<unsigned char> buf(1024 * 1024);
    if(std::fseek(ifp, header_size, SEEK_SET)){
        perror(orig_vcd);
        return -1;
    }
    while(!std::feof(ifp)){
        size_t copied;
        for(copied = 0; copied < buf.size() && !std::feof(ifp); ){
            copied += std::fread(&buf.front() + copied, 1, buf.size() - copied, ifp);
        }
        for(size_t written = 0; written < copied; ){
            written += std::fwrite(&buf.front() + written, 1, copied - written, ofp);
        }
    }
    return 0;

}

//! update the header in-place
//
//! @param dst start point of VCD to be modified
//! @param v_ data to be written
//! @param header_size VCD header size
int inplace_mod(void *dst, const std::vector<char> &v_, size_t header_size){
    std::vector<char> v = v_;
    const unsigned int fill_unit = 80;
    for(size_t cur = v.size(); cur < header_size; cur += fill_unit + 1){
        const unsigned int fill_size = std::min<unsigned int>(fill_unit + 1, header_size - cur);
        v.insert(v.end(), fill_size - 1, ' ');
        if(fill_size) v.push_back('\n');
    }
    assert(header_size == v.size());
    std::memcpy(dst, &v.front(), v.size());
    return 0;
}
 
} //end of unnamed namespace

int main(int argc, char *argv[]){
    bool flatten = false;
    std::string output_file;
    for(;;){
        struct option long_options[] = {
            {"flatten", 0, NULL, 0},
            {"output", 1, NULL, 1},
            {0, 0, 0, 0}
        };
        int opt_idx = -1;
        const int c = getopt_long(argc, argv, "", long_options, &opt_idx);
        if(c == -1) break;
        switch(opt_idx){
            case 0:
                flatten = true;
                break;
            case 1:
                output_file = optarg;
                break;
            default:
                std::cerr << "unknown option " << std::endl;
                return -1;
                break;
        }
    }
    if(optind >= argc){
        std::cerr << "Input file is not specified" << std::endl;
        return -1;
    }
    const char *vcd_filename = argv[optind];

    const size_t header_size = get_vcd_header_size(vcd_filename);
    mmap_manager vcd_file(vcd_filename, true, header_size);

    string_view all(static_cast<const char *>(vcd_file.get_ptr()), vcd_file.get_size());
    vcd_header *const orig = parse_vcd_header(all);
    //orig->dump(std::cout);
    if(flatten){
        for(int level = 0; level < 1; ++level){
            std::vector<char> v;
            orig->flatten(v, level);
            std::cerr << "Header size " << std::dec << header_size << " -> " << v.size() << std::endl;
            if(!output_file.empty()){
                return make_new_file_and_write(vcd_filename, output_file.c_str(), v, header_size);
            }
            else if(v.size() <= header_size){
                return inplace_mod(vcd_file.get_ptr(), v, header_size);
            }
        }
 
    }
    else{
        vcd_header *const hier = orig->make_hierarchy();
        //hier->dump(std::cout);
        for(int level = 0; level < 1; ++level){
            std::vector<char> v;
            hier->to_str(v, level);
            std::cerr << "Header size " << std::dec << header_size << " -> " << v.size() << std::endl;
            if(!output_file.empty()){
                return make_new_file_and_write(vcd_filename, output_file.c_str(), v, header_size);
            }
            else if(v.size() <= header_size){
                return inplace_mod(vcd_file.get_ptr(), v, header_size);
            }
        }
    }
    std::cerr
        << "Could not complete. Because modified header cannot be smaller than the original one.\n"
        << "Please add --output option" << std::endl;
    return 0;
}
