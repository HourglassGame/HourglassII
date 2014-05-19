//
//  GlobalOptions.h
//  HourglassII
//
//  Created by Evan Wallace on 7/05/2014.
//  Copyright (c) 2014 Team Causality. All rights reserved.
//

#ifndef HG_GLOBAL_OPTIONS_H
#define HG_GLOBAL_OPTIONS_H

//THIS IS NOT YET PROPERLY DESIGNED. Feel free to change it.
//IN FACT, IT DOESN'T EVEN DO ANYTHING YET!
#include <unordered_map>
#include <type_traits>
#include <SFML/Window/Keyboard.hpp>
namespace hg {
enum class Operation : int{
    PauseGame,
    GuyLeft,
    GuyRight
    //etc.
};
}
namespace std {
    template<>
    struct hash<hg::Operation> {
        std::size_t operator()(hg::Operation v) const {
            return std::hash<int>()(static_cast<int>(v));
        }
    };
}
namespace hg {
struct GlobalOptions {
    std::unordered_multimap<Operation, sf::Keyboard::Key> keyBinds;
};
}
#endif //HG_GLOBAL_OPTIONS_H
