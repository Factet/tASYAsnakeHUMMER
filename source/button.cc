#include <cstring>
#include "window.h"
#include "text.h"
#include "image.h"
#include "button.h"

#define BORDERSIZE 3

Button::Button(const uint32_t command)
{
    m_ClassName = __FUNCTION__;
    m_liteColor = RGB(0.99, 0.99, 0.99);
    m_darkColor = RGB(0.2, 0.2, 0.2);
    SetFrameWidth(1);
    SetCommand(command);
    m_bToggle = false;
    m_bToggleState = false;
}

Button::~Button()
{
}

void Button::SetCommand(const uint32_t command)
{
    m_command = command;
}

void Button::OnDraw(Context *cr)
{
    Rect ws = GetInteriorSize();
    uint16_t w = ws.GetWidth();
    uint16_t h = ws.GetHeight();

    // рисуем рельеф
    cr->SetLineWidth(1);

    for(unsigned short i=0; i<BORDERSIZE; i++)
    {
        cr->SetColor(!m_bToggleState ? m_liteColor : m_darkColor);

        // сверху
        cr->Line(Point(i,i),Point(w-i+1,i));
        // слева
        cr->Line(Point(i,i),Point(i,h-i+1));

        cr->SetColor(!m_bToggleState ? m_darkColor : m_liteColor);

        // снизу
        cr->Line(Point(i,h-i),Point(w-i+1,h-i));
        // справа
        cr->Line(Point(w-i,i),Point(w-i,h-i+1));
    }
}

bool Button::OnLeftMouseButtonClick(const Point &position)
{
    if(m_bToggle)
    {
        SetToggleState(!m_bToggleState);
        ReDraw();
    }
    NotifyParent(m_command,position);
    return true;
}

RGB  Button::GetLiteColor()
{
    return m_liteColor;
}

void Button::SetLiteColor(const RGB liteColor)
{
    m_liteColor = liteColor;
}

RGB  Button::GetDarkColor()
{
    return m_darkColor;
}

void Button::SetDarkColor(const RGB darkColor)
{
    m_darkColor = darkColor;
}

void Button::SetToggle(bool bToggle)
{
    // выключим кнопку, если была включена
    if(m_bToggle && !bToggle && m_bToggleState)
    {
        m_bToggleState = false;
        ReDraw();
    }
    m_bToggle = bToggle;
}

bool Button::GetToggle()
{
    return m_bToggle;
}

void Button::SetToggleState(bool bToggleState)
{
    m_bToggleState = bToggleState;
}

bool Button::GetToggleState()
{
    return m_bToggleState;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TextButton::TextButton(const char *text, const uint32_t command) : Button(command)
{
    m_ClassName = __FUNCTION__;
    m_textColor = RGB(0.0, 0.0, 0.0);
    m_fontSize = 14;
    strcpy(m_fontFace,"Cantarel");

    strncpy(m_text, text, sizeof(m_text)-1);
}

TextButton::~TextButton()
{
}

void TextButton::OnCreate()
{
    Rect size = GetInteriorSize();

    m_pText = new Text(m_text);
    m_pText->SetFont(m_fontFace, m_fontSize, -1, -1);
    m_pText->SetAlignment(TEXT_ALIGNH_CENTER|TEXT_ALIGNV_CENTER);
    m_pText->SetBackColor(GetBackColor());
    m_pText->SetFrameWidth(0);
    m_pText->SetTextColor(m_textColor);
    m_pText->SetWrap(true);

    AddChild(m_pText, Point(BORDERSIZE,BORDERSIZE),
        size-Rect(2*(BORDERSIZE),2*(BORDERSIZE)));
}

RGB  TextButton::GetTextColor()
{
    return m_textColor;
}

void TextButton::SetTextColor(const RGB textColor)
{
    m_textColor = textColor;
    m_pText->SetTextColor(m_textColor);
}

uint16_t TextButton::GetFontSize()
{
    return m_fontSize;
}

void     TextButton::SetFontSize(const uint16_t fontSize)
{
    m_fontSize = fontSize;
    m_pText->SetFont(m_fontFace, m_fontSize, -1, -1);
}

void TextButton::SetBackColor(const RGB backColor)
{
    Window::SetBackColor(backColor);
    m_pText->SetBackColor(backColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImageButton::ImageButton(const IMAGEINFO ii, const uint32_t command) : Button(command)
{
    m_ClassName = __FUNCTION__;
    m_pImage = new Image(ii);
}

ImageButton::~ImageButton()
{
}

void ImageButton::OnCreate()
{
    Rect size = GetInteriorSize();

    AddChild(m_pImage, Point(BORDERSIZE,BORDERSIZE), size-Rect(2*(BORDERSIZE),2*(BORDERSIZE)));
    m_pImage->SetStyle(IMAGE_SCALE_FIT | IMAGE_ALIGNH_CENTER | IMAGE_ALIGNV_CENTER);
}

void ImageButton::SetStyle(uint8_t style)
{
    m_pImage->SetStyle(style);
}

uint8_t ImageButton::GetStyle()
{
    return m_pImage->GetStyle();
}
void ImageButton::SetBackColor(const RGB backColor)
{
    Window::SetBackColor(backColor);
    m_pImage->SetBackColor(backColor);
}
