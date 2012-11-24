#include "LayeredCanvas.h"
#include "Canvas.h"
#include "clone_ptr.h"
#include <boost/range/algorithm/stable_sort.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>

namespace hg {

namespace lc_internal {
    class Drawer {
    public:
        virtual Drawer* clone() const = 0;
        virtual void drawTo(Canvas& canvas) const = 0;
        virtual ~Drawer(){}
    };
    class RectDrawer : public Drawer {
    public:
        RectDrawer(int x, int y, int width, int height, unsigned colour) :
            x(x), y(y), width(width), height(height), colour(colour)
        {
        }
        void drawTo(Canvas& canvas) const {
            canvas.drawRect(x, y, width, height, colour);
        }
        RectDrawer* clone() const {
            return new RectDrawer(*this);
        }
    private:
        int x;
        int y;
        int width;
        int height;
        unsigned colour;
    };
    class LineDrawer : public Drawer {
    public:
        LineDrawer(int xa, int ya, int xb, int yb, int width, unsigned colour) :
            xa(xa), ya(ya), xb(xb), yb(yb), width(width), colour(colour)
        {}
        void drawTo(Canvas& canvas) const {
            canvas.drawLine(xa, ya, xb, yb, width, colour);
        }
        LineDrawer* clone() const {
            return new LineDrawer(*this);
        }
    private:
        int xa;
        int ya;
        int xb;
        int yb;
        int width;
        unsigned colour;
    };
    class DrawCall {
    public:
        DrawCall(int layer, Drawer* drawer) :drawer(drawer), layer(layer)
        {
        }
        
        DrawCall(DrawCall const& o) : drawer(o.drawer), layer(o.layer)
        {}
        
        DrawCall& operator=(DrawCall const& o) {
            drawer = o.drawer;
            layer = o.layer;
            return *this;
        }
        
        bool operator<(DrawCall const& o) const {
            return layer < o.layer;
        }
        void drawTo(Canvas& canvas) const {
            drawer->drawTo(canvas);
        }
    private:
        clone_ptr<Drawer> drawer;
        int layer;
    };
}
using namespace lc_internal;
LayeredCanvas::LayeredCanvas(Canvas& canvas) : canvas(&canvas), drawCalls()
{
}
void LayeredCanvas::drawRect(int layer, int x, int y, int width, int height, unsigned colour) {
    drawCalls.push_back(DrawCall(layer, new RectDrawer(x, y, width, height, colour)));
}
void LayeredCanvas::drawLine(int layer, int xa, int ya, int xb, int yb, int width, unsigned colour) {
    drawCalls.push_back(DrawCall(layer, new LineDrawer(xa,ya,xb,yb,width,colour)));
}
void LayeredCanvas::flush() {
    boost::stable_sort(drawCalls);
    boost::for_each(drawCalls, boost::bind(&DrawCall::drawTo, _1, boost::ref(*canvas)));
}
LayeredCanvas::~LayeredCanvas(){}
}//namespace hg

