#include <cstring>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include "window.h"
#include "text.h"

Text::Text(const char *text)
{
    m_ClassName = __FUNCTION__;
    m_textColor = RGB(0.0, 0.0, 0.0);
    m_fontSize = 12;
    strcpy(m_fontFace,"Cantarel");

    m_text = nullptr;
    m_textSize = 0;
    if(text)
    {
        SetText(text);
    }
    m_style = TEXT_ALIGNH_LEFT|TEXT_ALIGNV_TOP;
    m_bWrap = false;
    m_nlines = 0;
    m_lines = nullptr;
}

Text::~Text()
{
    free(m_text);
    ClearLines();
}

RGB  Text::GetTextColor()
{
    return m_textColor;
}

void Text::SetTextColor(const RGB textColor)
{
    m_textColor = textColor;
}

void Text::GetFont(char **FontFace, uint16_t *FontSize, uint16_t *bold, uint16_t *italic)
{
    if(FontFace)
    {
        *FontFace = m_fontFace;
    }
    if(FontSize)
    {
        *FontSize = m_fontSize;
    }
    if(bold)
    {
        *bold = m_style & TEXT_STYLE_BOLD;
    }
    if(italic)
    {
        *italic = m_style & TEXT_STYLE_ITALIC;
    }
}

void Text::SetFont(const char *FontFace, const int16_t FontSize, const int16_t bold, const int16_t italic)
{
    if(FontFace)
    {
        assert(strlen(FontFace)<=FONTFACE_SIZE);
        strcpy(m_fontFace,FontFace);
    }
    if(FontSize >= 0)
    {
        m_fontSize = FontSize;
    }
    if(bold >= 0)
    {
        m_style = (m_style & ~TEXT_STYLE_BOLD) | (bold & TEXT_STYLE_BOLD);
    }
    if(italic >= 0)
    {
        m_style = (m_style & ~TEXT_STYLE_ITALIC) | (italic & TEXT_STYLE_ITALIC);
    }
}

uint8_t Text::GetAlignment()
{
    return m_style & (TEXT_ALIGNH_MASK|TEXT_ALIGNV_MASK);
}

void Text::SetAlignment(uint8_t mode)
{
    uint8_t h = mode & TEXT_ALIGNH_MASK;
    // если задан ровно один режим выравнивания по горизонтали
    if(h == TEXT_ALIGNH_LEFT || h == TEXT_ALIGNH_CENTER || h == TEXT_ALIGNH_RIGHT)
    {
        // установим его
        m_style = (m_style & ~TEXT_ALIGNH_MASK) | h;
    }

    uint8_t v = mode & TEXT_ALIGNV_MASK;
    // если задан ровно один режим выравнивания по вертикали
    if(v == TEXT_ALIGNV_TOP || v == TEXT_ALIGNV_CENTER || v == TEXT_ALIGNV_BOTTOM)
    {
        // установим его
        m_style = (m_style & ~TEXT_ALIGNV_MASK) | v;
    }
}

bool Text::GetWrap()
{
    return m_bWrap;
}

void Text::SetWrap(bool bWrap)
{
    m_bWrap = bWrap;
    m_bTextIsReady = false;
}



bool Text::SetText(const char *text)
{
    if(!CheckUTF8(text))
    {
        return false;
    }

    m_textSize = strlen(text);
    if(m_text == nullptr)
    {
        m_text = (char *) malloc(m_textSize+1);
    }
    else
    {
        m_text = (char *) realloc(m_text, m_textSize+1);
    }
    assert(m_text);
    strcpy(m_text, text);
    m_bTextIsReady = false;
    return true;
}

char *Text::GetText()
{
    return m_text;
}

void Text::OnDraw(Context *cr)
{
    cr->GetFontInfo(m_fontFace, m_fontSize, m_style, &m_ascent, &m_descent, &m_ls, &m_adv);

    Point ws = GetInteriorSize();

    uint16_t limit = m_bWrap ? ws.GetX() - 2*GetGap() : 0;
    if(!m_bTextIsReady)
    {
        PrepareLines(cr, limit);
        m_bTextIsReady = true;
    }

    // фон
    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(0,0), m_BackgroundSize);

    // положение базовой точки в окне текста зависит от требуемого выравнивания
    int16_t startX=0;
    switch(m_style & TEXT_ALIGNH_MASK)
    {
    case TEXT_ALIGNH_LEFT:
        startX = GetGap();
        break;
    case TEXT_ALIGNH_CENTER:
        startX = ws.GetX()/2;
        break;
    case TEXT_ALIGNH_RIGHT:
        startX = ws.GetX() - GetGap();
        break;
    }

    int16_t startY=0;
    switch(m_style & TEXT_ALIGNV_MASK)
    {
    case TEXT_ALIGNV_TOP:
        startY = 0;
        break;
    case TEXT_ALIGNV_CENTER:
        startY = (ws.GetY()-m_ls*(m_nlines-1))/2;
        break;
    case TEXT_ALIGNV_BOTTOM:
        startY = ws.GetY()-m_ls*(m_nlines-1);
        break;
    }

    // текст
    cr->SetColor(m_textColor);
    uint16_t advance, width = 0;
    for(uint16_t i=0; i<m_nlines; i++)
    {
        cr->Text(m_lines[i], m_fontFace, m_fontSize, Point(startX, startY+m_ls*i), m_style, &advance);
        width = max(width, advance);
    }

    ComputeDataRect(width, limit);
}

void Text::OnSizeChanged()
{
    m_bTextIsReady = false;
}

uint8_t Text::GetUnicodeSymbolSize(uint8_t firstbyte)
{
    if((firstbyte & 0x80) == 0x00)
    {
        // UTF-8: ASCII (8 bits)
        return 1;
    }
    else if((firstbyte & 0xe0) == 0xc0)
    {
        // UTF-8: 16 bits
        return 2;
    }
    else if((firstbyte & 0xf0) == 0xe0)
    {
        // UTF-8: 24 bits
        return 3;
    }
    else if((firstbyte & 0xf8) == 0xf0)
    {
        // UTF-8: 32 bits
        return 4;
    }
    else
    {
        // не должно быть
        assert(0);
        return 0;
    }
}

bool Text::CheckUTF8(const char *s)
{
    while(*s)
    {
        uint8_t l = GetUnicodeSymbolSize(s[0]);
        if(l==0)
        {
            return false;
        }
        for(uint8_t i=1; i<l; i++)
        {
            if((s[i]&0xc0) != 0x80)
            {
                return false;
            }
        }
        s+=l;
    }
    return true;
}

// подготовка текста: разбиение по строкам
void Text::PrepareLines(Context *cr, uint16_t limit)
{
    ClearLines();
    char tmp[m_textSize+1];             // рабочий буфер
    uint32_t pos=0;                     // позиция в исходном тексте
    uint16_t linepos=0;                 // позиция в очередной строке
    uint16_t width=0;                   // ширина текста

    if( limit > 0 && limit < m_adv)
    {
        return;
    }

    // память для первого элемента массива
    m_lines = (char **) malloc(sizeof(char *));
    m_nlines = 1;

    // по одному исследуем символы текста
    while(pos < m_textSize)
    {
        if(m_text[pos] == '\t')
        {
            // заменяем символ табуляции на пробел
            tmp[linepos] = ' ';
            ++pos;
            ++linepos;
        }
        else if(m_text[pos] == '\n')
        {
            // очередная строка закончена
            tmp[linepos] = 0;

            // память для нее и копируем
            m_lines[m_nlines-1] = (char *) malloc(linepos+1);
            strcpy(m_lines[m_nlines-1], tmp);

            // увеличим массив
            ++m_nlines;
            m_lines = (char **) realloc(m_lines, m_nlines*sizeof(char *));

            ++pos;
            linepos = 0;
        }
        else
        {
            // копируем очередной символ текста
            uint16_t s = GetUnicodeSymbolSize(m_text[pos]);
            for(uint16_t i=0; i<s; i++)
            {
                tmp[linepos++] = m_text[pos++];
            }

            // узнаем текущий размер строки
            tmp[linepos] = 0;
            uint16_t temp, advance;
            cr->GetTextInfo(tmp, m_fontFace, m_fontSize, m_style, &temp, &temp, &advance);
            if(limit == 0)
            {
                width = max(width, advance);
            }
            else if(advance > limit)
            {
                // выход за предел длины строки: надо убрать последний символ и перейти к новой строке
                pos -= s;
                linepos -= s;

                // очередная строка закончена
                tmp[linepos] = 0;

                // память для нее и копируем
                m_lines[m_nlines-1] = (char *) malloc(linepos+1);
                strcpy(m_lines[m_nlines-1], tmp);

                // увеличим массив
                ++m_nlines;
                m_lines = (char **) realloc(m_lines, m_nlines*sizeof(char *));

                linepos = 0;
            }
        }
    }

    if(linepos > 0)
    {
        // последняя строка
        tmp[linepos] = 0;

        // память для нее и копируем
        m_lines[m_nlines-1] = (char *) malloc(linepos+1);
        strcpy(m_lines[m_nlines-1], tmp);
    }
    else
    {
        --m_nlines;
    }

    ComputeDataRect(width, limit);
}

// вычисление размера данных
void Text::ComputeDataRect(const uint16_t width, const uint16_t limit)
{
    // размер области с текстом
    uint16_t w, h;
    Rect s = GetInteriorSize();

    w = (limit == 0 ? width : limit) + 2*GetGap();
    w = max(s.GetWidth(), w);
    h = m_nlines*m_ls;
    h = max(s.GetHeight(), h);

    bool bNotify = (w != m_BackgroundSize.GetWidth()) || (h != m_BackgroundSize.GetHeight());
    m_BackgroundSize.SetWidth(w);
    m_BackgroundSize.SetHeight(h);

    if(bNotify)
    {
        GetParent()->OnDataRectChanged(this,m_BackgroundSize);
    }
}


void Text::ClearLines()
{
    for(uint16_t i=0; i<m_nlines; i++)
    {
        free(m_lines[i]);
    }
    free(m_lines);
    m_nlines = 0;
    m_lines = nullptr;
}

Rect    &Text::GetDataRect()
{
    return m_BackgroundSize;//m_textDimensions;
}
