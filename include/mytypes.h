// mytypes.h

#ifndef max
    #define max(a,b) ((a)>(b) ? (a) : (b))
#endif

#ifndef min
    #define min(a,b) ((a)<(b) ? (a) : (b))
#endif

class Rect
{
public:
    Rect() : m_w(0), m_h(0) {}
    Rect(const uint16_t w, const uint16_t h) : m_w(w), m_h(h) {}
    Rect(const Rect &r) : m_w(r.GetWidth()), m_h(r.GetHeight()) {}
    ~Rect() {}

    Rect operator = (const Rect &r)
    {
        m_w = r.GetWidth();
        m_h = r.GetHeight();
        return *this;
    }
    Rect operator + (const Rect &r) const
    {
        Rect res(GetWidth()+r.GetWidth(), GetHeight()+r.GetHeight());
        return res;
    }
    Rect operator - (const Rect &r) const
    {
        Rect res(GetWidth()-r.GetWidth(), GetHeight()-r.GetHeight());
        return res;
    }

    uint16_t GetWidth() const { return m_w; }
    uint16_t GetHeight() const { return m_h; }
    void     SetWidth(const uint16_t w) { m_w = w; }
    void     SetHeight(const uint16_t h) { m_h = h; }

private:
    uint16_t m_w;
    uint16_t m_h;
};

class Point
{
public:
    Point() : m_x(0), m_y(0) {}
    Point(uint16_t x, uint16_t y) : m_x(x), m_y(y) {}
    Point(const Point &pt) : m_x(pt.GetX()), m_y(pt.GetY()) {}
    Point(const Rect &r) : m_x(r.GetWidth()), m_y(r.GetHeight()) {}
    ~Point() {}

    Point operator = (const Point &pt)
    {
        m_x = pt.GetX();
        m_y = pt.GetY();
        return *this;
    }
    Point operator + (const Point pt) const
    {
        Point res(GetX()+pt.GetX(), GetY()+pt.GetY());
        return res;
    }
    Point operator - (const Point pt) const
    {
        Point res(GetX()-pt.GetX(), GetY()-pt.GetY());
        return res;
    }

    Point operator + (const Rect &r) const
    {
        Point res(GetX()+r.GetWidth(), GetY()+r.GetHeight());
        return res;
    }
    Point operator - (const Rect &r) const
    {
        Point res(GetX()-r.GetWidth(), GetY()-r.GetHeight());
        return res;
    }

    uint16_t GetX() const { return m_x; }
    uint16_t GetY() const { return m_y; }
    void     SetX(const uint16_t x) { m_x = x; }
    void     SetY(const uint16_t y) { m_y = y; }

private:
    uint16_t m_x;
    uint16_t m_y;
};

class RGB
{
public:
    RGB() : m_red(0), m_green(0), m_blue(0) {}
    RGB(double red, double green, double blue)
    {
        m_red = red;
        m_green = green;
        m_blue = blue;
    }
    RGB(const RGB &clr) : m_red(clr.GetRed()), m_green(clr.GetGreen()), m_blue(clr.GetBlue()) {}

    double GetRed() const { return m_red; }
    double GetGreen() const { return m_green; }
    double GetBlue() const { return m_blue; }

private:
    double m_red, m_green, m_blue;
};

#define RGB_RED     RGB(1.0, 0.0, 0.0)
#define RGB_GREEN   RGB(0.0, 1.0, 0.0)
#define RGB_BLUE    RGB(0.0, 0.0, 1.0)
#define RGB_CYAN    RGB(0.0, 1.0, 1.0)
#define RGB_MAGENTA RGB(1.0, 0.0, 1.0)
#define RGB_YELLOW  RGB(1.0, 1.0, 0.0)
#define RGB_WHITE   RGB(1.0, 1.0, 1.0)
#define RGB_BLACK   RGB(0.0, 0.0, 0.0)


typedef void * IMAGEPTR;
typedef struct _IMAGEINFO
{
    IMAGEPTR imageptr;
    int32_t  width;
    int32_t  height;
} * IMAGEINFO;

typedef struct _SCROLLINFO
{
    enum {
        SCROLL_UNKNOWN = 1,
        SCROLL_UP,
        SCROLL_DOWN,
        SCROLL_LEFT,
        SCROLL_RIGHT,
        SCROLL_SMOOTH,
    } direction;
    bool   stop;
    double dx;
    double dy;
} * SCROLLINFO;

// коды управляющих символов ASCII
#define KEY_Backspace       0x08
#define KEY_Tab             0x09
#define KEY_Linefeed        0x0a
#define KEY_Return          0x0d
#define KEY_Esc             0x1b

// коды клавиш, управляющих положением курсора
// эти коды не могут быть вставлены в строку символов Unicode,
// так как это недействительные коды Unicode в UTF-8
#define KEY_Delete          0xff
#define KEY_Home            0x80
#define KEY_Left            0x81
#define KEY_Up              0x82
#define KEY_Right           0x83
#define KEY_Down            0x84
#define KEY_PgUp            0x85
#define KEY_PgDn            0x86
#define KEY_End             0x87


