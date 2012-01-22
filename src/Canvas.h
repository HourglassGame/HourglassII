#ifndef HG_CANVAS_H
#define HG_CANVAS_H
namespace hg {
    class Canvas {
    public:
        virtual void drawRect(int x, int y, int width, int height, unsigned colour) = 0;
    protected:
        ~Canvas() {}
    };
}//namespace hg
#endif //HG_CANVAS_H