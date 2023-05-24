#ifndef HG_LUA_MODULE_H
#define HG_LUA_MODULE_H
#include <vector>
#include <string>
#include <tuple>
namespace hg {
struct LuaModule final {
public:
    LuaModule(std::string name, std::vector<char> chunk)
        : name(std::move(name)), chunk(std::move(chunk)){}
    std::string name;
    std::vector<char> chunk;

    bool operator==(LuaModule const& o) const = default;
};
}
#endif // HG_LUA_MODULE_H
