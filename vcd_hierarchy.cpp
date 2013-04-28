#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include "mmap_manager.h"
#include "vcd_header.h"

namespace{
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

}

int main(int, char *[]){
    const char *const vcd_filelame = "a.vcd";
    const size_t header_size = get_vcd_header_size(vcd_filelame);
    mmap_manager vcd_file(vcd_filelame, true, header_size);

    string_view all(static_cast<const char *>(vcd_file.get_ptr()), vcd_file.get_size());
    vcd_header *const orig = parse_vcd_header(all);
    //orig->dump(std::cout);
    vcd_header *const hier = orig->make_hierarchy();
    //hier->dump(std::cout);
    for(int level = 0; level < 1; ++level){
        std::vector<char> v;
        hier->to_str(v, level);
        std::cerr << "Header size " << std::dec << header_size << " -> " << v.size() << std::endl;
        if(v.size() <= header_size){
            const unsigned int fill_unit = 80;
            for(size_t cur = v.size(); cur < header_size; cur += fill_unit + 1){
                const unsigned int fill_size = std::min<unsigned int>(fill_unit + 1, header_size - cur);
                v.insert(v.end(), fill_size - 1, ' ');
                if(fill_size) v.push_back('\n');
            }
            assert(header_size == v.size());
            std::memcpy(vcd_file.get_ptr(), &v.front(), v.size());
            break;
        }
    }
#if 0
    v.push_back('\0');
    std::cout << &v.front() << std::endl;
#endif
    return 0;
}
