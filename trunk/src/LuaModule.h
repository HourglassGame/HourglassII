#ifndef HG_LUA_MODULE_H
#define HG_LUA_MODULE_H
#include <vector>
#include <string>
namespace hg {
struct LuaModule {
    LuaModule(std::string name, std::vector<char> chunk)
        : name(std::move(name)), chunk(std::move(chunk)){}
    std::string name;
    std::vector<char> chunk;
};
}
#endif // HG_LUA_MODULE_H
