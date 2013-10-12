#ifndef VCD_HEADER_H
#define VCD_HEADER_H
#include <map>
#include <utility>
#include <vector>
#include <iosfwd>


//! simple string-like class, 
//
//! Memory is not allocated unlike std::string.
//! This class let you tread an existing memory fragment as a string.
class string_view{
    //! Head of string
    const char *ptr;
    //! Length of string, not include null character ('\0').
    //! This class does not assume null terminated memory fragment.
    size_t len;
    public:
    string_view(const char *, size_t);
    string_view();
    const char & operator[] (size_t)const;
    size_t size()const;
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

//! signal in VCD file
class vcd_signal{
    //! pointer to the module that contain this signal
    const vcd_module *parent;
    //! true if this is wire
    bool is_wire;
    //! bit width of this signal
    string_view width;
    //! symbol in VCD file
    string_view symbol;
    //! signal name
    string_view name;
    public:
    vcd_signal(const string_view &, const vcd_module *);
    const string_view &get_name()const;
    const string_view &get_width()const;
    const string_view &get_symbol()const;
    void set_name(const string_view &);
    void set_parent(const vcd_module *);
    void dump(std::ostream &, int)const;
    const vcd_module *get_parent()const;
    const char *get_type_str()const;
};

//! module (hierarchy unit) in VCD file
class vcd_module{
    //! type of map to manage sub modules
    //! key is an instance name of module and value is a pointer to the sub module
    typedef std::map<string_view, vcd_module *> mod_map_type;
    //! iterator of mod_map_type
    typedef mod_map_type::iterator mod_it;
    //! const_iterator of mod_map_type
    typedef mod_map_type::const_iterator mod_const_it;
    //! type of map to manage signals
    //! key is an instance name of signal and value is a pointer to the signal
    typedef std::map<string_view, vcd_signal *> sig_map_type;
    //! iterator of sig_map_type
    typedef sig_map_type::iterator sig_it;
    //! const_iterator of sig_map_type
    typedef sig_map_type::const_iterator sig_const_it;
    //! parent module of this module
    const vcd_module *parent;
    //! instance name of this module
    string_view name;
    //! signals that belong to this module
    sig_map_type signals;
    //! sub modules that belong to this module
    mod_map_type sub_modules;
    vcd_module(const string_view &, const vcd_module *);
    void make_hierarchy_internal();
    vcd_signal & add_signal(const vcd_signal &);
    void collect_signals(std::vector<const vcd_signal *> &)const;
    public:
    vcd_module(string_view &, const string_view &, vcd_module *);
    ~vcd_module();
    const string_view &get_name()const;
    vcd_module *make_hierarchy()const;
    void dump(std::ostream &, int)const;
    void to_str(std::vector<char> &, int, int)const;
    void flatten(std::vector<char> &, int)const;
    const vcd_module *get_parent()const;
};

//! header information of VCD
class vcd_header{
    //! map to manage top modules in VCD
    //! key is an instance name of module and value is a pointer to the module
    typedef std::map<string_view, vcd_module *> mod_map_type;
    //! iterator of mod_map_type
    typedef mod_map_type::iterator mod_it;
    //! const_iterator of mod_map_type
    typedef mod_map_type::const_iterator mod_const_it;
    //! $data field of VCD header
    string_view date;
    //! $version field of VCD header
    string_view version;
    //! $timescale field of VCD header
    string_view timescale;
    //! $comment field of VCD header
    string_view comment;
    //! top modules
    mod_map_type top_modules;
    vcd_header();
    public:
    explicit vcd_header(string_view &);
    ~vcd_header();
    vcd_header *make_hierarchy()const;
    vcd_header *flatten()const;
    void dump(std::ostream &)const;
    void to_str(std::vector<char> &, int)const;
    void flatten(std::vector<char> &, int)const;
};

vcd_header * parse_vcd_header(const string_view &);


#endif
