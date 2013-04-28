#include <cassert>
#include <iostream>
#include <iomanip>
#include "vcd_header.h"

namespace{
const char *const seperator = " \t\n";


string_view get_tok(const string_view &str, size_t offset, const char *sep){
    //assert(offset <= str.size());
    const char *start = NULL;
    for(size_t idx = offset, end = str.size(); idx < end; ++idx){
        const char *s;
        for(s = sep; *s != '\0'; ++s){
            if(str[idx] == *s){
                if(start)
                    return string_view(start, idx - offset - (start - &str[offset]));
                else{
                    break;
                }
            }
        }
        if(*s == '\0' && !start){
            start = &str[idx];
        }
    }
    return string_view(start, start ? str.size() - offset : 0);

}

string_view get_line(const string_view &str, size_t offset){
    return get_tok(str, offset, "\n");
}

string_view skip_begging_blank(const string_view &s){
    for(size_t i = 0; i < s.size(); ++i){
        if(s[i] != ' ') return string_view(&s[i], s.size() - i);
    }
    return string_view(&s[0] + s.size(), 0);
}

struct indent{
    const int i;
    const char c;
    explicit indent(int i) : i(i), c('\t'){}
    indent(int i, char c) : i(i), c(c){}
};

std::vector<char> & operator << (std::vector<char> &v, const indent &i){
    v.insert(v.end(), i.i, i.c);
    return v;
}
std::vector<char> & operator << (std::vector<char> &v, const string_view &s){
    v.insert(v.end(), &s[0], &s[0] + s.size());
    return v;
}
std::vector<char> & operator << (std::vector<char> &v, const char *s){
    while(*s != '\0') v.push_back(*(s++));
    return v;
}



} //end of unnamed namespace

string_view::string_view(const char *s, size_t l) : ptr(s), len(l){}

string_view::string_view() : ptr(NULL), len(0){}
const char & string_view::operator[] (size_t idx)const{
    assert(idx < len);
    return ptr[idx];
}

size_t string_view::size()const{
    return len;
}

/*
string_view & string_view::operator << (const string_view &other){
    assert(ptr + len == other.ptr);
    len += other.len;
    return *this;
}
*/


string_view::param_pair_t string_view::get_param(){
    string_view key;
    const char *val_start = NULL;
    size_t val_len = 0;

    for(string_view tok = get_tok(*this, 0, seperator); tok.size(); tok = get_tok(*this, tok.ptr - this->ptr + tok.size(), seperator)){
        if(tok[0] == '$'){
            if(!key.size()){
                assert(tok != "$end");
                key = tok;
            }
            else{
                assert(tok == "$end");
                const size_t new_offset = tok.ptr - this->ptr + tok.size();
                assert(len >= new_offset);
                ptr += new_offset;
                len -= new_offset;
                val_len = val_start ? (&tok[0] - val_start - 1) : 0;
                return std::make_pair(key, string_view(val_start, val_len));
            }
        }
        else{
            if(!val_start) val_start = &key[0] + key.size() + 1;
        }
    }
    ptr += len;
    len = 0;
    return param_pair_t();
}

string_view string_view::chomp()const{
    for(const char *c= ptr + len - 1; c >= ptr; --c){
        bool found = false;
        for(const char *s = seperator; *s != '\0'; ++s){
            if(*s == *c){
                found = true;
                break;
            }
        }
        if(!found){
            return string_view(ptr, c - ptr + 1);
        }
    }
    return string_view(ptr, 0);
}

std::ostream & operator << (std::ostream &os, const string_view &str){
    for(size_t i = 0; i < str.size(); ++i){
        os << str[i];
    }
    return os;
}

std::ostream & operator << (std::ostream &os, const string_view::param_pair_t &p){
    return os << "key:" << p.first << " val:'" << p.second << '\'';
}

bool operator == (const char *a, const string_view &b){
    size_t idx; 
    for(idx = 0; idx < b.size(); ++idx){
        if(a[idx] != b[idx]) return false;
    }
    return a[idx] == '\0';
}

bool operator == (const string_view &a, const char *b){
    return b == a;
}


bool operator != (const char *a, const string_view &b){
    return !(a == b);
}

bool operator != (const string_view &a, const char *b){
    return b != a;
}

bool operator < (const string_view &a, const string_view &b){
    for(size_t i = 0; i < std::min(a.size(), b.size()); ++i){
        if(a[i] != b[i]) return a[i] < b[i];
    }
    return a.size() < b.size();
}

vcd_signal::vcd_signal(const string_view &info, const vcd_module *parent) : parent(parent){

    int depth = 0;
    const char *name_start = NULL;
    for(string_view tok = get_tok(info, 0, seperator); tok.size(); tok = get_tok(info, &tok[0] - &info[0] + tok.size(), seperator)){
        switch(depth++){
            case 0:
                assert(tok == "wire");
                break;
            case 1:
                width = tok;
                break;
            case 2:
                symbol = tok;
                break;
            case 3:
                name_start = &tok[0];
                break;
        }
    }
    assert(symbol.size());
    assert(name_start);
    name = string_view(name_start, info.size() - (name_start - &info[0])).chomp();
//    std::cerr << "Signal '" << name << "' is created" << std::endl;
}

vcd_signal::vcd_signal(const string_view &name, const string_view &width, const string_view &symbol, vcd_module *parent) : parent(parent), width(width), symbol(symbol), name(name)
{
}
const string_view & vcd_signal::get_name()const{
    return name;
}
const string_view & vcd_signal::get_symbol()const{
    return symbol;
}
const string_view & vcd_signal::get_width()const{
    return width;
}

void vcd_signal::set_name(const string_view &s){
    name = s;
}

void vcd_signal::set_parent(const vcd_module *m){
    parent = m;
}

void vcd_signal::dump(std::ostream &os, int level)const{
    os
        << std::setw(level * 2) << std::setfill(' ') << ""
        << "name:\'" << name << "' "
        << "width:\'" << width << "' "
        << "symbol:\'" << symbol << "' "
        << '\n';
}

vcd_module::vcd_module(const string_view &name, const vcd_module *parent) : parent(parent), name(name){}

vcd_module::vcd_module(string_view &header_str, const string_view &name_arg, vcd_module *parent) : parent(parent){
    const string_view mod = get_tok(name_arg, 0, seperator);
    assert(mod == "module");
    name = get_tok(name_arg, mod.size(), seperator).chomp();
//    std::cerr << "Module '" << name << "' is created" << std::endl;
    for(string_view::param_pair_t param_pair = header_str.get_param(); header_str.size(); param_pair = header_str.get_param()){
        if(param_pair.first == "$scope"){
            vcd_module *const mod = new vcd_module(header_str, param_pair.second, this);
            assert(sub_modules.find(mod->get_name()) == sub_modules.end());
            sub_modules[mod->get_name()] = mod;
        }
        else if(param_pair.first == "$var"){
            vcd_signal *const sig = new vcd_signal(param_pair.second, this);
            assert(signals.find(sig->get_name()) == signals.end());
            signals[sig->get_name()] = sig;
        }
        else if(param_pair.first == "$upscope"){
            return;
        }
        else{
            std::cerr << "Warning Not supported parameter " << param_pair << std::endl;
        }
    }
}

vcd_module::~vcd_module(){
    for(mod_const_it i = sub_modules.begin(), end = sub_modules.end(); i != end; ++i){
        delete i->second;
    }
    for(sig_const_it i = signals.begin(), end = signals.end(); i != end; ++i){
        delete i->second;
    }
}

vcd_signal & vcd_module::add_signal(const string_view &name, const string_view &width, const string_view &symbol){
    assert(signals.find(name) == signals.end());
    vcd_signal *const s = new vcd_signal(name, width, symbol, this);
    signals[name] = s;
    return *s;
}

void vcd_module::make_hierarchy_internal(){
    for(sig_it i = signals.begin(), end = signals.end(); i != end; ){
        const string_view new_sub_mod_name = get_tok(i->second->get_name(), 0, ".");
        if(new_sub_mod_name.size() == i->second->get_name().size()){
            ++i;
        }
        else{
            const string_view new_signal_name = string_view(&(i->second->get_name()[new_sub_mod_name.size() + 1]), i->second->get_name().size() - (new_sub_mod_name.size() + 1));
            if(sub_modules.find(new_sub_mod_name) == sub_modules.end())
                sub_modules[new_sub_mod_name] = new vcd_module(new_sub_mod_name, this);
            vcd_module &sub_mod = *sub_modules[new_sub_mod_name];
            assert(sub_mod.signals.find(new_signal_name) == sub_mod.signals.end());
            i->second->set_name(new_signal_name);
            i->second->set_parent(&sub_mod);
            sub_mod.signals[new_signal_name] = i->second;
            signals.erase(i++);
        }
    }
    for(std::map<string_view, vcd_module *>::iterator i = sub_modules.begin(), end = sub_modules.end(); i != end; ++i){
        i->second->make_hierarchy_internal();
    }
}

const string_view & vcd_module::get_name()const{
    return name;
}

vcd_module * vcd_module::make_hierarchy()const{
    vcd_module *const new_mod = new vcd_module(*this);
    new_mod->signals.clear();
    new_mod->sub_modules.clear();
    for(mod_const_it i = sub_modules.begin(), end = sub_modules.end(); i != end; ++i){
        new_mod->sub_modules[i->second->get_name()] =  i->second->make_hierarchy();
        new_mod->sub_modules[i->second->get_name()]->parent = new_mod;
    }
    for(sig_const_it i = signals.begin(), end = signals.end(); i != end; ++i){
        new_mod->add_signal(i->second->get_name(), i->second->get_width(), i->second->get_symbol());
    }
    new_mod->make_hierarchy_internal();
    return new_mod;
}

void vcd_module::dump(std::ostream &os, int level)const{
    os
        << std::setw(level * 2) << std::setfill(' ') << ""
        << "name:\'" << name << "'\n"
        << std::setw(level * 2) << std::setfill(' ') << ""
        << "signals\n";
    for(std::map<string_view, vcd_signal *>::const_iterator i = signals.begin(), end = signals.end(); i != end; ++i){
        i->second->dump(os, level + 1);
    }
    os
        << std::setw(level * 2) << std::setfill(' ') << ""
        << "submodules\n";
    for(std::map<string_view, vcd_module *>::const_iterator i = sub_modules.begin(), end = sub_modules.end(); i != end; ++i){
        i->second->dump(os, level + 1);
    }

}

void vcd_module::to_str(std::vector<char> &dst, int size_level, int level)const{
    dst << indent(size_level <= 0 ? level : 0) << "$scope module " << name << " $end\n";
    for(sig_const_it i = signals.begin(), end = signals.end(); i != end; ++i){
        const vcd_signal &sig = *i->second;
        dst << indent(size_level <= 0 ? level + 1 : 0)
            << "$var wire " << sig.get_width()
            << " " << sig.get_symbol()
            << " " << sig.get_name()
            << " $end\n";
    }
    for(mod_const_it i = sub_modules.begin(), end = sub_modules.end(); i != end; ++i){
        i->second->to_str(dst, size_level, level + 1);
    }
    dst << indent(size_level <= 0 ? level : 0)
        << "$upscope $end\n";
}

vcd_header::vcd_header(string_view &header_str){
    struct{
        const char *const str;
        string_view &val;
    } const table[] = {
        {"$date", date},
        {"$version", version},
        {"$timescale", timescale},
        {"$comment", comment}
    };
    for(string_view::param_pair_t param_pair = header_str.get_param(); header_str.size(); param_pair = header_str.get_param()){
        //std::cout << param_pair << std::endl;
        if(param_pair.first.size() == 0) continue;
        bool found = false;
        for(size_t i = 0; i < sizeof(table)/sizeof(table[0]); ++i){
            if(table[i].str == param_pair.first){
                assert(table[i].val.size() == 0);
                table[i].val = param_pair.second;
//                std::cerr << "Set " << param_pair << std::endl;
                found = true;
                break;
            }
        }
        if(!found){
            if(param_pair.first == "$scope"){
                vcd_module *const mod = new vcd_module(header_str, param_pair.second, NULL);
                assert(top_modules.find(mod->get_name()) == top_modules.end());
                top_modules[mod->get_name()] = mod;
            }
            else if(param_pair.first == "$enddefinitions"){
//                return;
            }
            else{
                std::cerr << "Warning Not supported parameter " << param_pair << std::endl;
            }
        }
    }
//    assert(!"Never comes here");
}

vcd_header::~vcd_header(){
    for(mod_const_it i = top_modules.begin(), end = top_modules.end(); i != end; ++i){
        delete i->second;
    }
}

vcd_header * vcd_header::make_hierarchy()const{
    vcd_header *const new_header = new vcd_header(*this);
    new_header->top_modules.clear();
    for(mod_const_it i = top_modules.begin(), end = top_modules.end(); i != end; ++i){
        vcd_module *const top = i->second->make_hierarchy();
        assert(new_header->top_modules.find(top->get_name()) == new_header->top_modules.end());
        new_header->top_modules[top->get_name()] = top;
    }
    return new_header;
}

vcd_header * parse_vcd_header(const string_view &all){
    string_view header_str = all;
    vcd_header *const header = new vcd_header(header_str);
    return header;
}

void vcd_header::dump(std::ostream &os)const{
    os
        << "date:'" << date << "'\n"
        << "version:'" << version << "'\n"
        << "timescale:'" << timescale << "'\n"
        << "comment:'" << comment << "'\n"
        << "submodules\n"
        ;
    for(mod_const_it i = top_modules.begin(), end = top_modules.end(); i != end; ++i){
        i->second->dump(os, 1);
    }
}

void vcd_header::to_str(std::vector<char> &dst, int level)const{
    if(level < 1){
        dst << "$date\n" << date << "\n$end\n";
        if(level <= 0) dst << "\n";
        dst << "$version\n" << version << "\n$end\n";
        if(level <= 0) dst << "\n";
        dst << "$timescale\n" << timescale << "\n$end\n";
        if(level <= 0) dst << "\n";
    }
    for(mod_const_it i = top_modules.begin(), end = top_modules.end(); i != end; ++i){
        i->second->to_str(dst, level, 1);
    }
    dst << "$enddefinitions $end\n";
    if(level < 1){
        if(level <= 0) dst << "\n";
        dst << "$comment\n" << comment << "\n$end\n";
    }
}
