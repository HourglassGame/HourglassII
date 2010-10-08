#ifndef HG_MENU_H
#define HG_MENU_H
#include "Position.h"
#include "MenuItem.h"
#include "Utility.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <algorithm>
#define foreach BOOST_FOREACH

namespace hg {
template <typename HorizontalPositionPolicy, typename VerticalPositionPolicy>
class Menu {
public:
    Menu() :
    items()
    {
    }
    void addItem(MenuItem< ::sf::RenderTarget>* newItem)
    {
        newItem->setCentre(typename HorizontalPositionPolicy::SetCentreTagType(),
                           typename VerticalPositionPolicy::SetCentreTagType());
        items.push_back(newItem);
    }
    void draw(::sf::RenderTarget& target)
    {
        foreach(MenuItem< ::sf::RenderTarget>& item, items) {
            item.draw(target);
        }
    }
    void updatePositions(int newHeight, int newWidth)
    {
        float maxWidth(0.f);
        float totalHeight(0);
        const float kSpaceAroundItems(1.5f);
        foreach (const MenuItem< ::sf::RenderTarget>& item, items) {
            maxWidth = ::std::max(maxWidth, item.getXSize());
            //1.5f is to give a little room around the items
            totalHeight += kSpaceAroundItems * item.getYSize();
        }
        //want to take up less than 0.5 of a page both vertically and horizontally
        float scaleFactor(::std::min(newWidth*0.5f/maxWidth, newHeight*0.5f/totalHeight));
        maxWidth *= scaleFactor;
        totalHeight *= scaleFactor;
        foreach (MenuItem< ::sf::RenderTarget>& item, items) {
            item.scale(scaleFactor);
        }
        float yPosition(VerticalPositionPolicy::StartPosition(totalHeight, newHeight));
        float xPosition(HorizontalPositionPolicy::StartPosition(maxWidth, newWidth));
        
        foreach (MenuItem< ::sf::RenderTarget>& item, items) {
            item.setXPosition((int)xPosition);
            item.setYPosition((int)yPosition);
            yPosition += item.getYSize() * kSpaceAroundItems;
        }
    }
    bool updatePress(int mouseX, int mouseY)
    {
        foreach(MenuItem< ::sf::RenderTarget>& item, items) {
            if (item.updatePress(mouseX,mouseY)) {
                return true;//Death path, everything may have been deleted. Be Careful.
            }
        }
        return false;
    }
private:
    ::boost::ptr_vector<MenuItem< ::sf::RenderTarget> > items;
};
}
#endif //HG_MENU_H
