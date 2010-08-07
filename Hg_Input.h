#ifndef HG_INPUT_H
#define HG_INPUT_H
/*
 *  Hg_Input.h
 *  HourglassIIGameSFML
 *
 *  Created by Evan Wallace on 20/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
namespace hg {
    class Input;
}

#include <boost/noncopyable.hpp>
#include <SFML/Window/Input.hpp>
#define BOOST_SP_DISABLE_THREADS
#include <boost/smart_ptr/shared_ptr.hpp>
#ifndef INC_INPUTLIST
#define INC_INPUTLIST
#include "InputList.h"
#endif // INC_INPUTLIST

namespace hg {
    class Input : private boost::noncopyable {
    public:
        Input(const sf::Input& input);
        InputList AsInputList() const;
    private:
        bool left;
        bool right;
        bool up;
        bool down;
        bool space;
        bool mouseLeft;
        bool mouseRight;
        int mouseX;
        int mouseY;
    };
}
#endif //HG_INPUT_H
