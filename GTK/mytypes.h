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
    Rect(const int16_t w, const int16_t h) : m_w(w), m_h(h) {}
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

    friend std::ostream& operator <<(std::ostream &str, const Rect &p);

    int16_t GetWidth() const { return m_w; }
    int16_t GetHeight() const { return m_h; }
    void     SetWidth(const int16_t w) { m_w = w; }
    void     SetHeight(const int16_t h) { m_h = h; }

private:
    int16_t m_w;
    int16_t m_h;
};

class Point
{
public:
    Point() : m_x(0), m_y(0) {}
    Point(int16_t x, int16_t y) : m_x(x), m_y(y) {}
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

    friend std::ostream& operator <<(std::ostream &str, const Point &p);

    int16_t GetX() const { return m_x; }
    int16_t GetY() const { return m_y; }
    void     SetX(const int16_t x) { m_x = x; }
    void     SetY(const int16_t y) { m_y = y; }

private:
    int16_t m_x;
    int16_t m_y;
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
    uint8_t  *data;
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

