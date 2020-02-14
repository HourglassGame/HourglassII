#ifndef HG_LUA_MODULE_H
#define HG_LUA_MODULE_H
#include <vector>
#include <string>
#include <tuple>
namespace hg {
struct LuaModule final {
private:
    auto comparison_tuple() const -> decltype(auto)
    {
        return std::tie(name, chunk);
    }
public:
    LuaModule(std::string name, std::vector<char> chunk)
        : name(std::move(name)), chunk(std::move(chunk)){}
    std::string name;
    std::vector<char> chunk;

    bool operator==(LuaModule const& o) const
    {
        return comparison_tuple() == o.comparison_tuple();
    }
};
}
#endif // HG_LUA_MODULE_H
