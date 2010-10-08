#ifndef HG_MENU_ITEM_H
#define HG_MENU_ITEM_H
#include "Utility.h"
#include "Position.h"
namespace sf {
    class RenderTarget;
}
namespace hg {
//Knows its position and size, can have its position set, knows what to do if clicked
    template <class RenderTargetType> 
    class MenuItem {
    public:
        virtual void setCentre(horizontalposition::LEFT, verticalposition::TOP) = 0;
        virtual void setCentre(horizontalposition::LEFT, verticalposition::MIDDLE) = 0;
        virtual void setCentre(horizontalposition::LEFT, verticalposition::BOTTOM) = 0;
        virtual void setCentre(horizontalposition::MIDDLE, verticalposition::TOP) = 0;
        virtual void setCentre(horizontalposition::MIDDLE, verticalposition::MIDDLE) = 0;
        virtual void setCentre(horizontalposition::MIDDLE, verticalposition::BOTTOM) = 0;
        virtual void setCentre(horizontalposition::RIGHT, verticalposition::TOP) = 0;
        virtual void setCentre(horizontalposition::RIGHT, verticalposition::MIDDLE) = 0;
        virtual void setCentre(horizontalposition::RIGHT, verticalposition::BOTTOM) = 0;
        virtual void setCentre(float horizontalPosition, float verticalPosition) = 0;

        //returns bounding box size
        virtual float getXSize() const = 0;
        virtual float getYSize() const = 0;
        //Sets the position of the centre of the object, which can be set through calls to setCentre
        //(default centre is the centre of the object)
        virtual void setXPosition(int position) = 0;
        virtual void setYPosition(int position) = 0;
        //scales the x and y size. Does not affect calls to set*Position
        // (ie, setXPosition(600) will always put the centre at 600, not at 600*scalingFactor). 
        //affects calls to get*Size(so: assert(getXSize()==1000); scale(0.5f); assert(getXSize()==500))
        //Effect is cumulative (ie setScale(1.) is always a noop)
        virtual void scale(float scale) = 0;
        //should be called to signal that a mouse press occured at (pressX, pressY)
        //Takes appropriate action and returns true if that press activated the button
        virtual bool updatePress(int pressX, int pressY) = 0;
        virtual void draw(RenderTargetType& target) = 0;
        virtual ~MenuItem() = 0;
    };
    template <class RenderTargetType> MenuItem<RenderTargetType>::~MenuItem()
    {
    }
    template <class RenderTargetType, class DrawableType, class BoundActionType>
    class ActiveMenuItem : public MenuItem<RenderTargetType>
    {
    public:
        ActiveMenuItem(const DrawableType& nimage, const BoundActionType& action) :
        boundAction(action),
        image(nimage)
        {
            ::hg::setCentre(image, horizontalposition::MIDDLE(), verticalposition::MIDDLE());
        }
        void setCentre(horizontalposition::LEFT x, verticalposition::TOP y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(horizontalposition::LEFT x, verticalposition::MIDDLE y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(horizontalposition::LEFT x, verticalposition::BOTTOM y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(horizontalposition::MIDDLE x, verticalposition::TOP y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(horizontalposition::MIDDLE x, verticalposition::MIDDLE y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(horizontalposition::MIDDLE x, verticalposition::BOTTOM y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(horizontalposition::RIGHT x, verticalposition::TOP y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(horizontalposition::RIGHT x, verticalposition::MIDDLE y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(horizontalposition::RIGHT x, verticalposition::BOTTOM y)
        {
            ::hg::setCentre(image, x, y);
        }
        void setCentre(float horizontalPosition, float verticalPosition)
        {
            ::hg::setCentre(image, horizontalPosition, verticalPosition);
        }
        
        float getXSize() const
        {
            return ::hg::getXSize(image);
        }
        float getYSize() const
        {
            return ::hg::getYSize(image);
        }
        void setXPosition(int position)
        {
            ::hg::setXPosition(image, position);
        }
        void setYPosition(int position)
        {
            ::hg::setYPosition(image, position);
        }
        void scale(float nscale)
        {
            ::hg::scale(image, nscale);
        }
        bool updatePress(int pressX, int pressY)
        {
            float left(image.GetPosition().x-(image.GetCenter().x*image.GetScale().x));
            float top(image.GetPosition().y-(image.GetCenter().y*image.GetScale().y));
            float right(left + ::hg::getXSize(image));
            float bottom(top + ::hg::getYSize(image));

            if (pressX >= left &&
                pressX <= right &&
                pressY >= top &&
                pressY <= bottom) {
                boundAction();
                return true;
            }
            return false;
        }
        void draw(RenderTargetType& target)
        {
            ::hg::draw(target,image);
        }
        ~ActiveMenuItem()
        {
        }
        
    private:
        BoundActionType boundAction;
        DrawableType image;
    };
}
#endif //HG_MENU_ITEM_H
