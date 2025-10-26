#include "Text.h"


namespace MINTGGGameEngine
{

Text::Text(int16_t x, int16_t y, int size, Color color, String text)
    : d(std::make_shared<Data>())
{
    d->x = x;
    d->y = y;
    d->size = size;
    d->color = color;
    d->text = text;
    d->visible = true;
}

}
