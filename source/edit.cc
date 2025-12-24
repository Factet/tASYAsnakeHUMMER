#include <cstring>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include "window.h"
#include "edit.h"

Edit::Edit(const char *text)
{
    m_ClassName = __FUNCTION__;
    SetBackColor(RGB(1.0, 1.0, 1.0));
    m_activeColor = RGB(1.0, 0.0, 0.0);
    m_textColor = RGB(0.0, 0.0, 0.0);
    SetFrameWidth(2);
    m_fontSize = 12;
    strcpy(m_fontFace,"Cantarel");

    m_maxtextSize = TEXT_CHUNK_SIZE;
    m_text = (char *) malloc(TEXT_CHUNK_SIZE+1);
    m_shadow = (char *) malloc(TEXT_CHUNK_SIZE+1);
    m_text[0] = 0;
    m_shadow[0] = 0;
    m_textSize = 0;
    m_pointerPosition = 0;
    if(text)
    {
        SetText(text);
    }
    m_bFocus = false;
    m_bStoredClick = false;
    m_style = 0;
}

Edit::~Edit()
{
    free(m_text);
    free(m_shadow);
}

RGB  Edit::GetTextColor()
{
    return m_textColor;
}

void Edit::SetTextColor(const RGB textColor)
{
    m_textColor = textColor;
}

void Edit::GetFont(char **FontFace, uint16_t *FontSize, uint16_t *bold, uint16_t *italic)
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

void Edit::SetFont(const char *FontFace, const int16_t FontSize, const int16_t bold, const int16_t italic)
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



bool Edit::SetText(const char *text)
{
    if(!CheckUTF8(text))
    {
        return false;
    }
    uint32_t s = strlen(text);
    ExpandText(s);
    strcpy(m_text, text);
    strcpy(m_shadow, text);
    m_textSize = s;
    m_pointerPosition = GetTextLength();
    return true;
}

void Edit::ExpandText(uint32_t newsize)
{
    if(newsize > m_maxtextSize)
    {
        m_maxtextSize = newsize+TEXT_CHUNK_SIZE;
        m_text = (char *) realloc(m_text, m_maxtextSize+1);
        m_shadow = (char *) realloc(m_shadow, m_maxtextSize+1);
    }
}

char *Edit::GetText()
{
    return m_text;
}

void Edit::OnDraw(Context *cr)
{
    m_style |= TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER;
    cr->GetFontInfo(m_fontFace, m_fontSize, m_style, &m_ascent, &m_descent, &m_ls, &m_adv);

    Point ws = GetInteriorSize();

    // фон
    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(0,0), ws);

    // текст
    cr->SetColor(m_textColor);
    cr->Text(m_text, m_fontFace, m_fontSize, Point(m_adv/5, ws.GetY()/2), m_style);

    // мигающий указатель
    if(m_bFocus && m_Pointer>0)
    {
        DrawPointer(cr, ws);
    }
}

void Edit::DrawPointer(Context *cr, const Point &ws)
{
    uint16_t tmp, advance;
    uint16_t leftmargin = m_adv/5;

    if(m_bStoredClick)
    {
        uint16_t posX = m_StoredClick.GetX();
        uint16_t newpos, oldpos = 0;
        uint32_t s = 0;
        uint32_t maxpos = GetTextLength();
        uint32_t b = 0, old_b = 0;

        while(s<maxpos)
        {
            assert(m_text[b]);
            m_shadow[b] = 0;
            cr->GetTextInfo(m_shadow, m_fontFace, m_fontSize, m_style, &tmp, &tmp, &advance);

            newpos = advance + leftmargin;
            if(newpos > posX)
            {
                if((oldpos+newpos)/2 > posX)
                {
                    s = s>0 ? s-1 : 0;
                    m_shadow[old_b] = 0;
                }
                break;
            }

            old_b = b;
            CopyUnicodeSymbol(b);
            ++s;
            oldpos = newpos;
        }
        m_shadow[b] = 0;

        m_pointerPosition = s;
        m_bStoredClick = false;
    }

    cr->GetTextInfo(m_shadow, m_fontFace, m_fontSize, m_style, &tmp, &tmp, &advance);

    cr->SetLineWidth(1);
    Point from(leftmargin+advance,(ws.GetY()-(m_ascent+m_descent))/2);
    Point to(leftmargin+advance,(ws.GetY()+m_ascent+m_descent)/2);
    cr->Line(from,to);
}

bool Edit::OnLeftMouseButtonClick(const Point &position)
{
    // фокус ввода
    CaptureKeyboard(this);
    m_StoredClick = position;
    m_bStoredClick = true;
    m_Pointer = 2;
    ReDraw();
    return true;
}

bool Edit::OnKeyPress(uint64_t value)
{
//    std::cout << m_ClassName << "::OnKeyPress(" << value << ")" << std::endl;
    if(value < 32)                          // управляющие символы
    {
        if(value == KEY_Backspace)
        {
            if(m_pointerPosition>0)
            {
                SetPointer(m_pointerPosition-1);
                Delete();
                ReDraw();
            }
        }
        else if(value == KEY_Esc)
        {
            CaptureKeyboard(GetParent());
        }
    }
    else if(value < 1<<7)                   // ASCII
    {
        Insert(value,1);
    }
    else if(value < 1<<8)                   // управляющие символы, не UTF-8
    {
        uint32_t newPos = m_pointerPosition;
        switch(value)
        {
        case KEY_Home:
            newPos = 0;
            break;
        case KEY_End:
            newPos = GetTextLength();
            break;
        case KEY_Left:
            newPos = newPos > 0 ? newPos-1 : 0;
            break;
        case KEY_Right:
            newPos = newPos < GetTextLength() ? newPos+1 : GetTextLength();
            break;
        case KEY_Delete:
            if(m_pointerPosition < GetTextLength())
            {
                Delete();
            }
            break;
        default:
            return true;
        }

        SetPointer(newPos);
        ReDraw();
    }
    else if(value < 1<<16)                  // UTF-8: 16 битов
    {
        Insert(value,2);
    }
    else if(value < 1<<24)                  // UTF-8: 24 бита
    {
        Insert(value,3);
    }
    else if(value < ((uint64_t)1)<<32)      // UTF-8: 32 бита
    {
        Insert(value,4);
    }
    else
    {
        assert(0);                          // не должно быть
    }
    return true;
}

bool Edit::OnKeyboardCapture(bool bCapture)
{
//    std::cout << m_ClassName << "::OnKeyboardCapture(" << bCapture << ")" << std::endl;
    bool bSave = m_bFocus;
    m_bFocus = bCapture;
    if(bSave != m_bFocus)
    {
        // управление отображением указателя
        if(!bSave && m_bFocus)
        {
            // указатель надо показать
            CreateTimeout(this, POINTER_TIMEOUT);
            m_Pointer = 1;
        }

        if(bCapture)
        {
            m_saveColor = GetFrameColor();
            SetFrameColor(m_activeColor);
        }
        else
        {
            SetFrameColor(m_saveColor);
        }

        ReDraw();
    }
    return true;
}

bool Edit::OnTimeout()
{
    if(!m_bFocus)
    {
        // гасим указатель
        m_Pointer = 0;
    }
    if(m_Pointer>0)
    {
        // уменьшим значение
        --m_Pointer;
    }
    else
    {
        // включаем указатель
        m_Pointer = 1;
    }
    ReDraw();
    return m_bFocus;
}

uint32_t Edit::GetTextLength()
{
    uint32_t len = 0, i;

    for(i=0; i<m_textSize; i++)
    {
        if((m_text[i]&0xc0) != 0x80)
        {
            // начало символа Unicode
            ++len;
        }
    }

    return len;
}

// установка указателя при помощи стрелок, Home, End
void Edit::SetPointer(uint32_t newPosition)
{
    uint32_t b = 0;
    for(uint32_t s=0; s<newPosition; s++)
    {
        assert(m_text[b]);
        CopyUnicodeSymbol(b);
    }

    m_shadow[b] = 0;

    m_pointerPosition = newPosition;
    m_Pointer = 2;
}

void Edit::Insert(uint64_t value, uint8_t s)
{
    assert(s>0 && s<=4);
    ExpandText(m_textSize+s);

    // найдем смещение текущей позиции в байтах
    int32_t byteindex = GetByteIndex(m_pointerPosition);

    // сдвигаем правую часть строки вправо на s байтов
    for(int32_t i=m_textSize; i>=byteindex; i--)
    {
        m_text[i+s] = m_text[i];
    }

    // вставляем символ
    if(s == 1)
    {
        char c = value;
        m_text[byteindex] = c;
    }
    else if(s == 2)
    {
        unsigned char c1 = value>>8;
        unsigned char c2 = value&0xff;
        m_text[byteindex] = c1;
        m_text[byteindex+1] = c2;
    }
    else if(s == 3)
    {
        unsigned char c1 = value>>16;
        unsigned char c2 = (value>>8)&0xff;
        unsigned char c3 = value&0xff;
        m_text[byteindex] = c1;
        m_text[byteindex+1] = c2;
        m_text[byteindex+2] = c3;
    }
    else
    {
        unsigned char c1 = value>>24;
        unsigned char c2 = (value>>16)&0xff;
        unsigned char c3 = (value>>8)&0xff;
        unsigned char c4 = value&0xff;
        m_text[byteindex] = c1;
        m_text[byteindex+1] = c2;
        m_text[byteindex+2] = c3;
        m_text[byteindex+3] = c4;
    }
    m_textSize += s;
    SetPointer(m_pointerPosition+1);
    ReDraw();
}

void Edit::Delete()
{
    // найдем смещение текущей позиции в байтах
    int32_t byteindex = GetByteIndex(m_pointerPosition);

    // узнаем длину символа
    uint8_t s = GetUnicodeSymbolSize(m_text[byteindex]);

    // сдвигаем правую часть строки влево на s байтов
    for(int32_t i=byteindex; i<m_textSize; i++)
    {
        m_text[i] = m_text[i+s];
    }
    m_textSize -= s;
    ReDraw();
}

uint32_t Edit::GetByteIndex(uint32_t position)
{
    uint32_t byteindex = 0, symindex = 0;
    while(symindex < position)
    {
        assert(byteindex<m_textSize);
        // в зависимости от длины символа в байтах
        uint8_t s = GetUnicodeSymbolSize(m_text[byteindex]);
        byteindex += s;
        ++symindex;
    }

    return byteindex;
}

// копирование одного символа UTF-8 из m_text в m_shadow
void Edit::CopyUnicodeSymbol(uint32_t &b)
{
    uint8_t s = GetUnicodeSymbolSize(m_text[b]);
    for(uint8_t i=0; i<s; i++)
    {
        m_shadow[b] = m_text[b];
        ++b;
    }
}

uint8_t Edit::GetUnicodeSymbolSize(uint8_t firstbyte)
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

bool Edit::CheckUTF8(const char *s)
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
