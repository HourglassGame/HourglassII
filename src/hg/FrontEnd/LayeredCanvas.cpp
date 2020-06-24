#include "LayeredCanvas.h"
#include "Canvas.h"
#include "hg/Util/clone_ptr.h"
#include <boost/range/algorithm/stable_sort.hpp>
#include <algorithm>

namespace hg {

namespace lc_internal {
    class Drawer {
    public:
        virtual Drawer *clone() const = 0;
        virtual void drawTo(Canvas &canvas) const = 0;
        virtual ~Drawer() noexcept {}
    };
    class RectDrawer : public Drawer {
    public:
        RectDrawer(float x, float y, float width, float height, unsigned colour) :
            x(x), y(y), width(width), height(height), colour(colour)
        {
        }
        void drawTo(Canvas &canvas) const override {
            canvas.drawRect(x, y, width, height, colour);
        }
        RectDrawer *clone() const override {
            return new RectDrawer(*this);
        }
    private:
        float x;
        float y;
        float width;
        float height;
        unsigned colour;
    };
    class CircleDrawer : public Drawer {
    public:
        CircleDrawer(float x, float y, float radius, unsigned colour) :
            x(x), y(y), radius(radius), colour(colour)
        {
        }
        void drawTo(Canvas &canvas) const override {
            canvas.drawCircle(x, y, radius, colour);
        }
        CircleDrawer *clone() const override {
            return new CircleDrawer(*this);
        }
    private:
        float x;
        float y;
        float radius;
        unsigned colour;
    };
    class LineDrawer : public Drawer {
    public:
        LineDrawer(float xa, float ya, float xb, float yb, float width, unsigned colour) :
            xa(xa), ya(ya), xb(xb), yb(yb), width(width), colour(colour)
        {}
        void drawTo(Canvas &canvas) const override {
            canvas.drawLine(xa, ya, xb, yb, width, colour);
        }
        LineDrawer *clone() const override {
            return new LineDrawer(*this);
        }
    private:
        float xa;
        float ya;
        float xb;
        float yb;
        float width;
        unsigned colour;
    };
    class TextDrawer : public Drawer {
    public:
        TextDrawer(std::string const &text, float x, float y, float size, unsigned colour) :
            text(text), x(x), y(y), size(size), colour(colour)
        {}
        void drawTo(Canvas &canvas) const override {
            canvas.drawText(text, x, y, size, colour);
        }
        TextDrawer *clone() const override {
            return new TextDrawer(*this);
        }
    private:
        std::string text;
        float x;
        float y;
        float size;
        unsigned colour;
    };
    class ImageDrawer : public Drawer {
    public:
        ImageDrawer(std::string const &key, float x, float y, float width, float height) :
            key(key), x(x), y(y), width(width), height(height)
        {
        }
        void drawTo(Canvas &canvas) const override {
            canvas.drawImage(key, x, y, width, height);
        }
        ImageDrawer *clone() const override {
            return new ImageDrawer(*this);
        }
    private:
        std::string key;
        float x;
        float y;
        float width;
        float height;
    };
    class DrawCall {
    public:
        bool operator<(DrawCall const &o) const {
            return layer < o.layer;
        }
        void drawTo(Canvas &canvas) const {
            drawer->drawTo(canvas);
        }
        int layer;
        clone_ptr<Drawer> drawer;
    };
}
using namespace lc_internal;
LayeredCanvas::LayeredCanvas(Canvas &canvas) : canvas(&canvas), drawCalls()
{
}
void LayeredCanvas::playSound(std::string const &key, int n) {
    canvas->playSound(key, n);
}
void LayeredCanvas::drawRect(int layer, float x, float y, float width, float height, unsigned colour) {
    drawCalls.push_back(DrawCall{layer, make_clone_ptr<RectDrawer>(x, y, width, height, colour)});
}
void LayeredCanvas::drawCircle(int layer, float x, float y, float radius, unsigned colour) {
    drawCalls.push_back(DrawCall{layer, make_clone_ptr<CircleDrawer>(x, y, radius, colour)});
}
void LayeredCanvas::drawLine(int layer, float xa, float ya, float xb, float yb, float width, unsigned colour) {
    drawCalls.push_back(DrawCall{layer, make_clone_ptr<LineDrawer>(xa,ya,xb,yb,width,colour)});
}
void LayeredCanvas::drawText(int layer, std::string const &text, float x, float y, float size, unsigned colour) {
    drawCalls.push_back(DrawCall{layer, make_clone_ptr<TextDrawer>(text,x,y,size,colour)});
}
void LayeredCanvas::drawImage(int layer, std::string const &key, float x, float y, float width, float height) {
    drawCalls.push_back(DrawCall{layer, make_clone_ptr<ImageDrawer>(key, x, y, width, height)});
}
Flusher LayeredCanvas::getFlusher() {
    return Flusher(this);
}

void Flusher::partialFlush(int upperLimit) {
    boost::container::vector<lc_internal::DrawCall>::iterator lowerBound(upperBound);
    upperBound = std::upper_bound(lowerBound, boost::end(canvas->drawCalls), DrawCall{upperLimit, clone_ptr<Drawer>(nullptr)});
    std::for_each(lowerBound, upperBound, [this](lc_internal::DrawCall const &drawCall) { drawCall.drawTo(*canvas->canvas); });
}
Flusher::Flusher(LayeredCanvas *canvas) : canvas(canvas) {
    boost::stable_sort(canvas->drawCalls);
    upperBound = boost::begin(canvas->drawCalls);
}

LayeredCanvas::~LayeredCanvas() noexcept {}
}//namespace hg

