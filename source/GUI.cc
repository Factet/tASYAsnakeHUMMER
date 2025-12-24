#include <iostream>
#include <assert.h>
#include <cstring>

#include "window.h"
#include "GUI.h"

CairoContext::CairoContext()
{
    m_width = 1;
    m_x = 0;
    m_y = 0;
    m_xp = 0;
    m_yp = 0;
    m_stackCurSize = 0;
    m_stackMaxSize = 1;
    m_stack = (uint16_t*) malloc(m_stackMaxSize*2*sizeof(uint16_t));
}

CairoContext::~CairoContext()
{
    free(m_stack);
}

void CairoContext::SetCairoContext(cairo_t *cr)
{
    m_cr = cr;
    cairo_scale(m_cr, 1.0, 1.0);
//    m_cr2 = cairo_create(cairo_surface_create_similar(cairo_get_target(m_cr),CAIRO_CONTENT_COLOR,100,100));
}

void CairoContext::Save()
{
    cairo_save(m_cr);

    if(m_stackCurSize == m_stackMaxSize)
    {
        m_stackMaxSize += 1;
        m_stack = (uint16_t*) realloc(m_stack, m_stackMaxSize*2*sizeof(uint16_t));
    }

    m_stack[2*m_stackCurSize] = m_x;
    m_stack[2*m_stackCurSize+1] = m_y;

    ++m_stackCurSize;
}

void CairoContext::Restore()
{
    assert(m_stackCurSize>0);
    --m_stackCurSize;
    m_x = m_stack[2*m_stackCurSize];
    m_y = m_stack[2*m_stackCurSize+1];

    cairo_restore(m_cr);
}

void CairoContext::SetMask(const Point &pos, const Rect &size)
{
    cairo_rectangle(m_cr, pos.GetX()-m_x, pos.GetY()-m_y, size.GetWidth()+m_x, size.GetHeight()+m_y);
    cairo_clip(m_cr);
}

void CairoContext::SetPosition(const Point &pos)
{
    m_xp = pos.GetX();
    m_yp = pos.GetY();
}

void CairoContext::SetOrigin(const Point &pos)
{
    m_x += pos.GetX();
    m_y += pos.GetY();
}

void CairoContext::SetColor(const RGB &clr)
{
    m_color = clr;
}

void CairoContext::SetLineWidth(uint16_t width)
{
    m_width = width;
}

void CairoContext::FillRectangle(const Point &from, const Point &rectsize)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
	cairo_rectangle(m_cr, from.GetX()-m_x+m_xp, from.GetY()-m_y+m_yp, rectsize.GetX(), rectsize.GetY());
    cairo_fill(m_cr);
}

void CairoContext::FillRectangle(const Point &from, const Rect &rectsize)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
	cairo_rectangle(m_cr, from.GetX()-m_x+m_xp, from.GetY()-m_y+m_yp, rectsize.GetWidth(), rectsize.GetHeight());
    cairo_fill(m_cr);
}

void CairoContext::Rectangle(const Point &from, const Point &rectsize)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
	cairo_rectangle(m_cr, from.GetX()-m_x+m_xp, from.GetY()-m_y+m_yp, rectsize.GetX(), rectsize.GetY());
    cairo_stroke(m_cr);
}

void CairoContext::Rectangle(const Point &from, const Rect &rectsize)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
	cairo_rectangle(m_cr, from.GetX()-m_x+m_xp, from.GetY()-m_y+m_yp, rectsize.GetWidth(), rectsize.GetHeight());
    cairo_stroke(m_cr);
}

void CairoContext::Line(const Point &from, const Point &to)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
    cairo_move_to(m_cr, from.GetX()-m_x+m_xp, from.GetY()-m_y+m_yp);
    cairo_line_to(m_cr, to.GetX()-m_x+m_xp, to.GetY()-m_y+m_yp);
    cairo_stroke(m_cr);
}

void CairoContext::Text(const char *text, const char *fontface,
    const uint16_t fontsize, const Point &pt, const uint32_t style, uint16_t *advance)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
    cairo_select_font_face (m_cr, fontface,
        style & TEXT_STYLE_ITALIC ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
        style & TEXT_STYLE_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (m_cr, fontsize);

    double x,y;
    cairo_text_extents_t te;
    cairo_font_extents_t fe;
    cairo_text_extents (m_cr, text, &te);
    cairo_font_extents (m_cr, &fe);


    // выравнивание по горизонтали
    if(style & TEXT_ALIGNH_LEFT)
    {
        x = pt.GetX();
    }
    else if(style & TEXT_ALIGNH_RIGHT)
    {
        x = pt.GetX() - te.x_advance;
    }
    else // TEXT_ALIGNH_CENTER
    {
        x = pt.GetX() - te.x_advance / 2;
    }

    // выравнивание по вертикали
    if(style & TEXT_ALIGNV_TOP)
    {
        y = pt.GetY() + fe.ascent;
    }
    else if(style & TEXT_ALIGNV_BOTTOM)
    {
        y = pt.GetY() - fe.descent;
    }
    else // TEXT_ALIGNV_CENTER
    {
        y = pt.GetY() + (fe.ascent-fe.descent)/2;
    }

    cairo_move_to (m_cr, x-m_x+m_xp, y-m_y+m_yp);
    cairo_show_text (m_cr, text);

    if(advance)
    {
        *advance = te.x_advance;
    }
}

void CairoContext::GetTextInfo(const char *text, const char *fontface,
    const uint16_t fontsize, const uint32_t style, uint16_t *width, uint16_t *height, uint16_t *advance)
{
    cairo_text_extents_t te;
    cairo_select_font_face (m_cr, fontface,
        style & TEXT_STYLE_ITALIC ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
        style & TEXT_STYLE_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (m_cr, fontsize);
    cairo_text_extents (m_cr, text, &te);

    *width = te.width;
    *height = te.height;
    *advance = te.x_advance;
}

void CairoContext::GetFontInfo(const char *fontface, const uint16_t fontsize, const uint32_t style,
    int16_t *ascent, int16_t *descent, uint16_t *linespacing, uint16_t *maxadvance)
{
    cairo_font_extents_t fe;
    cairo_select_font_face (m_cr, fontface,
        style & TEXT_STYLE_ITALIC ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
        style & TEXT_STYLE_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (m_cr, fontsize);
    cairo_font_extents (m_cr, &fe);

    *ascent = fe.ascent;
    *descent = fe.descent;
    *linespacing = fe.height;
    *maxadvance = fe.max_x_advance;
}

void CairoContext::Polyline(const uint16_t n, const Point p[])
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);

	if(n > 1)
    {
        cairo_move_to(m_cr, p[0].GetX()-m_x+m_xp, p[0].GetY()-m_y+m_yp);
        for(uint16_t i=1; i<n; i++)
        {
            cairo_line_to(m_cr, p[i].GetX()-m_x+m_xp, p[i].GetY()-m_y+m_yp);
        }
        cairo_stroke(m_cr);
    }
}

void CairoContext::FillPolyline(const uint16_t n, const Point p[])
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);

	if(n > 1)
    {
        cairo_move_to(m_cr, p[0].GetX()-m_x+m_xp, p[0].GetY()-m_y+m_yp);
        for(uint16_t i=1; i<n; i++)
        {
            cairo_line_to(m_cr, p[i].GetX()-m_x+m_xp, p[i].GetY()-m_y+m_yp);
        }
        cairo_fill(m_cr);
    }
}

IMAGEINFO CairoContext::LoadPNG(const char *filename)
{
    cairo_surface_t *image;
    image = cairo_image_surface_create_from_png (filename);
    if(cairo_surface_status(image) != CAIRO_STATUS_SUCCESS)
    {
        return NULL;
    }

    IMAGEINFO ii = new struct _IMAGEINFO;
    ii->imageptr = (IMAGEPTR) image;
    ii->width = cairo_image_surface_get_width(image);
    ii->height = cairo_image_surface_get_height(image);
    return ii;
}

void CairoContext::DeletePNG(IMAGEINFO ii)
{
    cairo_surface_t *image = (cairo_surface_t *) ii->imageptr;
    cairo_surface_destroy (image);
    delete ii;
}

void CairoContext::Image(const IMAGEINFO ii, const Point &position, double scaleX, double scaleY)
{
    cairo_surface_t *image = (cairo_surface_t *) ii->imageptr;

    cairo_save(m_cr);
    cairo_scale (m_cr, scaleX, scaleY);

    cairo_set_source_surface (m_cr, image, (position.GetX()-m_x+m_xp)/scaleX, (position.GetY()-m_y+m_yp)/scaleY);
    cairo_paint (m_cr);
    cairo_restore(m_cr);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

GtkPlus *theGUI = nullptr;        // указатель на единственный объект приложения

GtkPlus::GtkPlus()
{
    m_ClassName = __FUNCTION__;
    m_pKeyboardOwner = nullptr;
    m_pMouseOwner = nullptr;
    m_bReDraw = false;
    assert(theGUI == nullptr);
    theGUI = this;
}

GtkPlus::~GtkPlus()
{
    theGUI = nullptr;
}

int GtkPlus::Run(int argc, char **argv, Window *wnd, uint16_t w, uint16_t h)
{
    // инициализация GTK+
	gtk_init(&argc, &argv);

	// Создаем главное окно, устанавливваем заголовок
	char title[80];
	strcpy(title, "GUI Model: ");
	strcat(title, argv[0]);
	m_Widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	m_Window = wnd;
	gtk_window_set_title(GTK_WINDOW(m_Widget), title);
	gtk_window_set_default_size (GTK_WINDOW(m_Widget),w,h);
//	gtk_window_fullscreen(GTK_WINDOW(m_Widget));

	SetSize(Rect(w,h));
	wnd->SetSize(Rect(w,h));

	wnd->Create(this);

	// Соединяем сигналы
	g_signal_connect(G_OBJECT(m_Widget), "destroy", G_CALLBACK(on_destroy), this);
	g_signal_connect(G_OBJECT(m_Widget), "draw", G_CALLBACK(on_draw), this);
	g_signal_connect(G_OBJECT(m_Widget), "size-allocate", G_CALLBACK(on_size_allocate), this);
    g_signal_connect (m_Widget, "button-press-event", G_CALLBACK (on_button_press_event), this);
    g_signal_connect (m_Widget, "button-release-event", G_CALLBACK (on_button_press_event), this);
    g_signal_connect (m_Widget, "key-press-event", G_CALLBACK (on_key_press_event), this);
    g_signal_connect (m_Widget, "motion-notify-event", G_CALLBACK (on_motion_notify_event), this);
    g_signal_connect (m_Widget, "scroll-event", G_CALLBACK (on_scroll_event), this);

    gtk_widget_set_events (m_Widget, gtk_widget_get_events (m_Widget)
        | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);

	// Показываем окно
	gtk_widget_show_all(m_Widget);

	// Приложение переходит в вечный цикл ожидания действий пользователя
	gtk_main();

	return 0;
}

bool GtkPlus::NotifyWindow(uint32_t type, const Point &p, uint64_t value, Window *pTarget)
{
    Window *pWindow = pTarget != NULL ? pTarget : m_Window;
    bool res = pWindow->WindowProc(type, p, value);

    if(m_bReDraw)
    {
        gtk_widget_queue_draw(m_Widget);
        m_bReDraw = false;
    }

    if(m_Window->m_bToBeDeleted)
    {
        gtk_main_quit();
        m_Window->DeleteAllChildren();
    }

    return res;
}

void GtkPlus::DestroyWidget(GtkWidget *widget)
{
    assert(m_Widget == widget);

    gtk_main_quit();
    m_Window->DeleteAllChildren();
}

gboolean GtkPlus::Draw(GtkWidget *widget, cairo_t *cr)
{
    assert(m_Widget == widget);
    SetCairoContext(cr);
    m_Window->Draw(this);
    return TRUE;
}

void GtkPlus::ReDraw()
{
    m_bReDraw = true;
}

void GtkPlus::CreateTimeout(Window *pWindow, uint32_t timeout)
{
    g_timeout_add(timeout, &on_timeout, pWindow);
}

void GtkPlus::CaptureKeyboard(Window *pWindow)
{
    if(m_pKeyboardOwner != pWindow)
    {
        if(m_pKeyboardOwner != NULL)
        {
            NotifyWindow(EVENT_KEYBOARDRELEASE, Point(0,0), 0, m_pKeyboardOwner);
        }
        m_pKeyboardOwner = pWindow;
        if(m_pKeyboardOwner != NULL)
        {
            NotifyWindow(EVENT_KEYBOARDCAPTURE, Point(0,0), 0, m_pKeyboardOwner);
        }
    }
}

void GtkPlus::CaptureMouse(Window *pWindow)
{
    m_pMouseOwner = pWindow;
}

bool GtkPlus::HasKeyboard(Window *pWindow)
{
    return m_pKeyboardOwner == pWindow;
}

gboolean GtkPlus::Allocation(GtkWidget *widget, GdkRectangle *allocation)
{
    assert(m_Widget == widget);
    return NotifyWindow(EVENT_WINDOWRESIZE, Point(allocation->width,allocation->height),0);
}

gboolean GtkPlus::ButtonPressEvent(GtkWidget *widget, GdkEventButton *event)
{
    assert(m_Widget == widget);

    uint32_t type;

    if(event->type == GDK_BUTTON_PRESS)
    {
        if(event->button == GDK_BUTTON_PRIMARY)
        {
            type = EVENT_LEFTMOUSEBUTTONCLICK;
        }
        else if(event->button == GDK_BUTTON_SECONDARY)
        {
            type = EVENT_RIGHTMOUSEBUTTONCLICK;
        }
        else
        {
            type = EVENT_UNKNOWN;
        }
    }
    else if(event->type == GDK_BUTTON_RELEASE)
    {
        if(event->button == GDK_BUTTON_PRIMARY)
        {
            type = EVENT_LEFTMOUSEBUTTONRELEASE;
        }
        else if(event->button == GDK_BUTTON_SECONDARY)
        {
            type = EVENT_RIGHTMOUSEBUTTONRELEASE;
        }
        else
        {
            type = EVENT_UNKNOWN;
        }
    }
    else if(event->type == GDK_2BUTTON_PRESS)
    {
        if(event->button == GDK_BUTTON_PRIMARY)
        {
            type = EVENT_LEFTMOUSEBUTTONDOUBLECLICK;
        }
        else if(event->button == GDK_BUTTON_SECONDARY)
        {
            type = EVENT_RIGHTMOUSEBUTTONDOUBLECLICK;
        }
        else
        {
            type = EVENT_UNKNOWN;
        }
    }
    else
    {
        type = EVENT_UNKNOWN;
    }

    return NotifyWindow(type, Point(event->x,event->y),0,m_pMouseOwner);
}

gboolean GtkPlus::MotionNotifyEvent(GtkWidget *widget, GdkEventMotion *event)
{
    assert(m_Widget == widget);

    uint32_t type;

    switch(event->type)
    {
    case GDK_MOTION_NOTIFY:
        type = EVENT_MOUSEMOVE;
        break;
    default:
        type = EVENT_UNKNOWN;
    }

    return NotifyWindow(type, Point(event->x,event->y),0,m_pMouseOwner);
}

gboolean GtkPlus::ScrollNotifyEvent(GtkWidget *widget, GdkEventScroll *event)
{
    assert(m_Widget == widget);

    uint32_t type;

    switch(event->type)
    {
    case GDK_SCROLL:
        type = EVENT_SCROLL;
        break;
    default:
        type = EVENT_UNKNOWN;
    }

    struct _SCROLLINFO si;
    si.dx = event->delta_x;
    si.dy = event->delta_y;

    switch(event->direction)
    {
    case GDK_SCROLL_DOWN:
        si.direction = _SCROLLINFO::SCROLL_DOWN;
        si.stop = true;
        break;
    case GDK_SCROLL_LEFT:
        si.direction = _SCROLLINFO::SCROLL_LEFT;
        si.stop = true;
        break;
    case GDK_SCROLL_RIGHT:
        si.direction = _SCROLLINFO::SCROLL_RIGHT;
        si.stop = true;
        break;
    case GDK_SCROLL_UP:
        si.direction = _SCROLLINFO::SCROLL_UP;
        si.stop = true;
        break;
    case GDK_SCROLL_SMOOTH:
        si.direction = _SCROLLINFO::SCROLL_SMOOTH;
        si.stop = event->is_stop;
        break;
    default:
        si.direction = _SCROLLINFO::SCROLL_UNKNOWN;
    }

    return NotifyWindow(type, Point(event->x,event->y), (uint64_t) &si);
}


gboolean GtkPlus::KeyPressEvent(GtkWidget *widget, GdkEventKey *event)
{
    assert(m_Widget == widget);
    unsigned char *str = (unsigned char *) event->string;
    uint64_t value = 0;
    for(unsigned int i=0; i<event->length; i++)
    {
        value <<= 8;
        value += str[i];
    }

    // перекодируем коды клавиш
    switch(event->keyval)
    {
    case GDK_KEY_BackSpace:
        value = KEY_Backspace;
        break;
    case GDK_KEY_Tab:
        value = KEY_Tab;
        break;
    case GDK_KEY_Linefeed:
        value = KEY_Linefeed;
        break;
    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
        value = KEY_Return;
        break;
    case GDK_KEY_Escape:
        value = KEY_Esc;
        break;
    case GDK_KEY_Delete:
    case GDK_KEY_KP_Delete:
        value = KEY_Delete;
        break;
    case GDK_KEY_Home:
    case GDK_KEY_KP_Home:
        value = KEY_Home;
        break;
    case GDK_KEY_Left:
    case GDK_KEY_KP_Left:
        value = KEY_Left;
        break;
    case GDK_KEY_Up:
    case GDK_KEY_KP_Up:
        value = KEY_Up;
        break;
    case GDK_KEY_Right:
    case GDK_KEY_KP_Right:
        value = KEY_Right;
        break;
    case GDK_KEY_Down:
    case GDK_KEY_KP_Down:
        value = KEY_Down;
        break;
    case GDK_KEY_Page_Up:
    case GDK_KEY_KP_Page_Up:
        value = KEY_PgUp;
        break;
    case GDK_KEY_Page_Down:
    case GDK_KEY_KP_Page_Down:
        value = KEY_PgDn;
        break;
    case GDK_KEY_End:
    case GDK_KEY_KP_End:
        value = KEY_End;
        break;
    case GDK_KEY_KP_Equal:
        value = '=';
        break;
    case GDK_KEY_KP_Multiply:
        value = '*';
        break;
    case GDK_KEY_KP_Add:
        value = '+';
        break;
    case GDK_KEY_KP_Subtract:
        value = '-';
        break;
    case GDK_KEY_KP_Divide:
        value = '/';
        break;
    case GDK_KEY_KP_0:
    case GDK_KEY_KP_1:
    case GDK_KEY_KP_2:
    case GDK_KEY_KP_3:
    case GDK_KEY_KP_4:
    case GDK_KEY_KP_5:
    case GDK_KEY_KP_6:
    case GDK_KEY_KP_7:
    case GDK_KEY_KP_8:
    case GDK_KEY_KP_9:
        value = event->keyval - GDK_KEY_KP_0 + '0';
        break;
    default:
        ;
    }

    return m_pKeyboardOwner != NULL ? NotifyWindow(EVENT_KEYPRESS, Point(0,0), value, m_pKeyboardOwner) : false;
}

void GtkPlus::Print()
{
    const char * separator = "------------------";
    assert(m_Window);
    std::cout << separator << std::endl;;
    uint32_t n = m_Window->PrintWindow();
    std::cout << "Total: " << n << " window(s)" << std::endl << separator << std::endl;;
}



int Run(int argc, char **argv, Window *wnd, uint16_t w, uint16_t h)
{
    GtkPlus *gui = new GtkPlus;
    int res = gui->Run(argc, argv, wnd, w, h);
    delete gui;
    return res;
}

void on_destroy(GtkWidget *widget, gpointer user_data)
{
    GtkPlus *gui = reinterpret_cast<GtkPlus*>(user_data);
    gui->DestroyWidget(widget);
}

gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    GtkPlus *gui = reinterpret_cast<GtkPlus*>(user_data);
    return gui->Draw(widget,cr);
}

gboolean on_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data)
{
    GtkPlus *gui = reinterpret_cast<GtkPlus*>(user_data);
    return gui->Allocation(widget,allocation);
}

gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    GtkPlus *gui = reinterpret_cast<GtkPlus*>(user_data);
    return gui->ButtonPressEvent(widget,event);
}

gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    GtkPlus *gui = reinterpret_cast<GtkPlus*>(user_data);
    return gui->KeyPressEvent(widget,event);
}

gboolean on_motion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    GtkPlus *gui = reinterpret_cast<GtkPlus*>(user_data);
    return gui->MotionNotifyEvent(widget,event);
}

gboolean on_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
    GtkPlus *gui = reinterpret_cast<GtkPlus*>(user_data);
    return gui->ScrollNotifyEvent(widget,event);
}

gboolean on_timeout(gpointer user_data)
{
    Window *win = reinterpret_cast<Window*>(user_data);
    assert(theGUI);
    return theGUI->NotifyWindow(EVENT_TIMEOUT, Point(0,0), 0, win);
}
