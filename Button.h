#ifndef HG_BUTTON_H
#define HG_BUTTON_H
#include "TimeDirection.h"
namespace hg
{
    class Button
    {

    public:

        Button(int x, int y, int attachment, int index, bool state, TimeDirection timeDirection);
        ~Button();
        Button(const Button& other);
        Button& operator=(const Button& other);

        inline int getX() const {return data->x;}
        inline int getY() const {return data->y;}
        inline int getAttachment() const {return data->attachment;}
        inline int getIndex() const {return data->index;}
        inline bool getState() const {return data->state;}
        inline TimeDirection getTimeDirection() const {return data->timeDirection;}

        bool operator==(const Button& other) const;
        bool operator!=(const Button& other) const;

        bool operator<(const Button& second) const;

    private:
        void decrementCount();

        struct Data;
        mutable int* referenceCount;
        Data* data;

        struct Data {
            Data(int nx,
                 int ny,
                 int nattachment,
                 int nindex,
                 bool nstate,
                 TimeDirection ntimeDirection) :
            x(nx),
            y(ny),
            attachment(nattachment),
            index(nindex),
            state(nstate),
            timeDirection(ntimeDirection)
            {
            }

            int x;
            int y;
            int attachment;
            int index;
            bool state;
            TimeDirection timeDirection;
        };
    };
}
#endif //HG_BUTTON_H
