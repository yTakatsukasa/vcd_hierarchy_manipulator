#ifndef VCD_HEADER_H
#define VCD_HEADER_H
#include <map>
#include <vector>
#include <iosfwd>


class string_view{
    const char *ptr;
    size_t len;
    public:
    string_view(const char *, size_t);
    string_view();
    const char & operator[] (size_t)const;
    size_t size()const;
    //string_view & operator << (const string_view &);
    typedef std::pair<string_view, string_view> param_pair_t;
    param_pair_t get_param();
    string_view chomp()const;
};

string_view get_line(const string_view &, size_t = 0);
std::ostream & operator << (std::ostream &, const string_view &);

bool operator == (const char *, const string_view &);
bool operator == (const string_view &, const char *);
bool operator != (const char *, const string_view &);
bool operator != (const string_view &, const char *);
bool operator < (const string_view &, const string_view &);


class vcd_header;
class vcd_module;

class vcd_signal{
    const vcd_module *parent;
    string_view width;
    string_view symbol;
    string_view name;
    public:
    vcd_signal(const string_view &, const vcd_module *);
    vcd_signal(const string_view &, const string_view &, const string_view &, vcd_module *);
    const string_view &get_name()const;
    const string_view &get_width()const;
    const string_view &get_symbol()const;
    void set_name(const string_view &);
    void set_parent(const vcd_module *);
    void dump(std::ostream &, int)const;
};

class vcd_module{
    typedef std::map<string_view, vcd_module *> mod_map_type;
    typedef mod_map_type::iterator mod_it;
    typedef mod_map_type::const_iterator mod_const_it;
    typedef std::map<string_view, vcd_signal *> sig_map_type;
    typedef sig_map_type::iterator sig_it;
    typedef sig_map_type::const_iterator sig_const_it;
    const vcd_module *parent;
    string_view name;
    sig_map_type signals;
    mod_map_type sub_modules;
    vcd_module(const string_view &, const vcd_module *);
    void make_hierarchy_internal();
    vcd_signal & add_signal(const string_view &name, const string_view &widh, const string_view &symbol);
    public:
    vcd_module(string_view &, const string_view &, vcd_module *);
    ~vcd_module();
    const string_view &get_name()const;
    vcd_module *make_hierarchy()const;
    void dump(std::ostream &, int)const;
    void to_str(std::vector<char> &, int, int)const;
};

class vcd_header{
    typedef std::map<string_view, vcd_module *> mod_map_type;
    typedef mod_map_type::iterator mod_it;
    typedef mod_map_type::const_iterator mod_const_it;
    string_view date;
    string_view version;
    string_view timescale;
    string_view comment;
    mod_map_type top_modules;
    vcd_header();
    public:
    explicit vcd_header(string_view &);
    ~vcd_header();
    vcd_header *make_hierarchy()const;
    vcd_header *flatten()const;
    void dump(std::ostream &)const;
    void to_str(std::vector<char> &, int)const;
};

vcd_header * parse_vcd_header(const string_view &);


#endif
