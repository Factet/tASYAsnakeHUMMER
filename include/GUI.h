#include <gtk/gtk.h>

class CairoContext : public Context
{
public:
    CairoContext();
    ~CairoContext();

    void SetCairoContext(cairo_t *cr);

    // унаследованные от класса Context методы
    virtual void Save();
    virtual void Restore();
    virtual void SetMask(const Point &pos, const Rect &size);
    virtual void SetPosition(const Point &pos);
    virtual void SetOrigin(const Point &pos);

    virtual void SetColor(const RGB &clr);
    virtual void SetLineWidth(uint16_t width);

    virtual void FillRectangle(const Point &from, const Point &to);
    virtual void FillRectangle(const Point &from, const Rect &size);
    virtual void Rectangle(const Point &from, const Point &to);
    virtual void Rectangle(const Point &from, const Rect &size);
    virtual void Line(const Point &from, const Point &to);
    virtual void Text(const char *text, const char *fontface,
        const uint16_t fontsize, const Point &pt, const uint32_t style, uint16_t *advance=0);
    virtual void GetTextInfo(const char *text, const char *fontface,
        const uint16_t fontsize, const uint32_t style, uint16_t *width,
        uint16_t *height, uint16_t *advance);
    virtual void GetFontInfo(const char *fontface, const uint16_t fontsize, const uint32_t style,
        int16_t *ascent, int16_t *descent, uint16_t *linespacing, uint16_t *maxadvance);
    virtual void Polyline(const uint16_t n, const Point p[]);
    virtual void FillPolyline(const uint16_t n, const Point p[]);
    IMAGEINFO LoadPNG(const char *filename);
    virtual void DeletePNG(IMAGEINFO imageptr);
    virtual void Image(const IMAGEINFO imageptr, const Point &position, double scaleX, double scaleY);

private:
    cairo_t  *m_cr;
    RGB      m_color;
    uint16_t m_width;
    uint16_t m_x, m_y;
    uint16_t m_xp, m_yp;
    uint16_t *m_stack;
    uint16_t m_stackMaxSize, m_stackCurSize;
};

class GtkPlus : public CairoContext, public Window
{
public:
    GtkPlus();
    ~GtkPlus();

    int Run(int arc, char **argv, Window *wnd, uint16_t w, uint16_t h);
    bool NotifyWindow(uint32_t type, const Point &p, uint64_t value, Window *pTarget=nullptr);

    void ReDraw();
    void CreateTimeout(Window *pWindow, uint32_t timeout);
    void CaptureKeyboard(Window *pWindow);
    void CaptureMouse(Window *pWindow);
    bool HasKeyboard(Window *pWindow);

    void DestroyWidget(GtkWidget *widget);
    gboolean Draw(GtkWidget *widget, cairo_t *cr);
    gboolean Allocation(GtkWidget *widget, GdkRectangle *allocation);
    gboolean ButtonPressEvent(GtkWidget *widget, GdkEventButton *event);
    gboolean KeyPressEvent(GtkWidget *widget, GdkEventKey *event);
    gboolean MotionNotifyEvent(GtkWidget *widget, GdkEventMotion *event);
    gboolean ScrollNotifyEvent(GtkWidget *widget, GdkEventScroll *event);

    void Print();

private:
    GtkWidget *m_Widget;
    Window    *m_Window;
    Window    *m_pKeyboardOwner;
    Window    *m_pMouseOwner;
    bool      m_bReDraw;
};

extern GtkPlus *theGUI;        // указатель на единственный объект приложения
int Run(int arc, char **argv, Window *wnd, uint16_t w, uint16_t h);

void on_destroy(GtkWidget *widget, gpointer user_data);
gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean on_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
gboolean on_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
gboolean on_timeout(gpointer user_data);
