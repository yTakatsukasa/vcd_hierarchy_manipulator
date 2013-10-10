#ifndef MMAP_MANAGER_H
#define MMAP_MANAGER_H

//! Manages the memory mapped file
class mmap_manager{
    struct impl;
    impl *pimpl;
    public:
    mmap_manager(const char *, bool);
    mmap_manager(const char *, bool, size_t);
    ~mmap_manager();
    void *get_ptr()const;
    size_t get_size()const;
};

#endif
