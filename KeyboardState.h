#ifndef HG_KEYBOARDSTATE_H
#define HG_KEYBOARDSTATE_H
/*
 *  KeyboardState.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include <vector>

namespace hg {
    struct KeyboardState 
    {
    public:
        KeyboardState(const char* keys, int keyShifts);
        const int key_shifts;
        inline const std::vector<char>& getKeys()
        {
            return key;
        }
    private:
        std::vector<char> key;
        
    };
}
#endif //HG_KEYBOARDSTATE_H
