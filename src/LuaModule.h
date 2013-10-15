#ifndef HG_LUA_MODULE_H
#define HG_LUA_MODULE_H
#include <vector>
#include <string>
namespace hg {
struct LuaModule {
    LuaModule(std::string const &name, std::vector<char> const &chunk) : name(name), chunk(chunk){}
    std::string name;
    std::vector<char> chunk;
};
}
#endif // HG_LUA_MODULE_H
