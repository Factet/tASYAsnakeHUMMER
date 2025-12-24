#include <cmath>
#include "window.h"
#include "digit7.h"

Digit7::Digit7(uint8_t d)
{
    m_ClassName = __FUNCTION__;
    SetDigit(d);
    m_Color = RGB(1,1,1);
    m_gap = 0.01;
    m_thickness = 0.1;
}

Digit7::~Digit7()
{
}

RGB Digit7::GetColor()
{
    return m_Color;
}

void Digit7::SetColor(RGB Color)
{
    m_Color = Color;
}

double Digit7::GetGap()
{
    return m_gap;
}

void Digit7::SetGap(double gap)
{
    m_gap = gap;
}

double Digit7::GetThickness()
{
    return m_thickness;
}

void Digit7::SetThickness(double thickness)
{
    m_thickness = thickness;
}

uint8_t Digit7::GetDigit()
{
    return m_digit;
}

void Digit7::SetDigit(uint8_t d)
{
    m_digit = d < 10 ? d : 0;
}

void Digit7::OnDraw(Context *cr)
{
    Rect mysize = GetInteriorSize();
    uint16_t x = mysize.GetWidth(), y = mysize.GetHeight();
    uint16_t s = x < y ? x : y;
    Point pts[6];
    double b = 0.5*m_thickness;
    double f = sqrt(2.0)*0.5*m_gap;

    const static bool data[10][7] =
    {
        {  true,  true,  true, false,  true,  true,  true },    // 0            0
        { false, false,  true, false, false,  true, false },    // 1          -----
        {  true, false,  true,  true,  true, false,  true },    // 2       1 |     | 2
        {  true, false,  true,  true, false,  true,  true },    // 3         |  3  |
        { false,  true,  true,  true, false,  true, false },    // 4          -----
        {  true,  true, false,  true, false,  true,  true },    // 5       4 |     | 5
        {  true,  true, false,  true,  true,  true,  true },    // 6         |  6  |
        {  true, false,  true, false, false,  true, false },    // 7          -----
        {  true,  true,  true,  true,  true,  true,  true },    // 8
        {  true,  true,  true,  true, false,  true,  true },    // 9
    };

    // цикл по сегментам
    for(uint8_t i=0; i<7; i++)
    {
        if(data[m_digit][i])
        {
            // очередной сегмент
            switch(i)
            {
            case 0:
                pts[0] = Point((b+f)*s, b*s);
                pts[1] = Point((2*b+f)*s, 2*b*s);
                pts[2] = Point(x-(2*b+f)*s, 2*b*s);
                pts[3] = Point(x-(b+f)*s, b*s);
                pts[4] = Point(x-(2*b+f)*s, 0);
                pts[5] = Point((2*b+f)*s, 0);
                break;
            case 1:
                pts[0] = Point(b*s, (b+f)*s);
                pts[1] = Point(2*b*s, (2*b+f)*s);
                pts[2] = Point(2*b*s, y/2-(b+f)*s);
                pts[3] = Point(b*s, y/2-f*s);
                pts[4] = Point(0, y/2-(b+f)*s);
                pts[5] = Point(0, (2*b+f)*s);
                break;
            case 2:
                pts[0] = Point(x-b*s, (b+f)*s);
                pts[1] = Point(x-2*b*s, (2*b+f)*s);
                pts[2] = Point(x-2*b*s, y/2-(b+f)*s);
                pts[3] = Point(x-b*s, y/2-f*s);
                pts[4] = Point(x, y/2-(b+f)*s);
                pts[5] = Point(x, (2*b+f)*s);
                break;
            case 3:
                pts[0] = Point((b+f)*s, y/2);
                pts[1] = Point((2*b+f)*s, y/2+b*s);
                pts[2] = Point(x-(2*b+f)*s, y/2+b*s);
                pts[3] = Point(x-(b+f)*s, y/2);
                pts[4] = Point(x-(2*b+f)*s, y/2-b*s);
                pts[5] = Point((2*b+f)*s, y/2-b*s);
                break;
            case 4:
                pts[0] = Point(b*s, y-(b+f)*s);
                pts[1] = Point(2*b*s, y-(2*b+f)*s);
                pts[2] = Point(2*b*s, y/2+(b+f)*s);
                pts[3] = Point(b*s, y/2+f*s);
                pts[4] = Point(0, y/2+(b+f)*s);
                pts[5] = Point(0, y-(2*b+f)*s);
                break;
            case 5:
                pts[0] = Point(x-b*s, y-(b+f)*s);
                pts[1] = Point(x-2*b*s, y-(2*b+f)*s);
                pts[2] = Point(x-2*b*s, y/2+(b+f)*s);
                pts[3] = Point(x-b*s, y/2+f*s);
                pts[4] = Point(x, y/2+(b+f)*s);
                pts[5] = Point(x, y-(2*b+f)*s);
                break;
            case 6:
                pts[0] = Point((b+f)*s, y-b*s);
                pts[1] = Point((2*b+f)*s, y-2*b*s);
                pts[2] = Point(x-(2*b+f)*s, y-2*b*s);
                pts[3] = Point(x-(b+f)*s, y-b*s);
                pts[4] = Point(x-(2*b+f)*s, y);
                pts[5] = Point((2*b+f)*s, y);
                break;
            default:
                ;
            }

            cr->SetColor(m_Color);
            cr->FillPolyline(6, pts);
        }
    }
}
