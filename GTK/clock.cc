#include <cmath>
#include <ctime>
#include <iostream>
#include <cassert>

#include <gtk/gtk.h>

#include "mytypes.h"
#include "context.h"

#define GAP 20

gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event);
void on_destroy(GtkWidget *widget, gpointer user_data);
gboolean on_timeout(gpointer user_data);
gboolean draw_area(GtkWidget *widget, cairo_t *cairo, gpointer user_data);
void draw_digit7(unsigned int digit, Context *cr, Rect &r);

// функция main
int main(int argc, char **argv)
{
	// инициализация GTK
	gtk_init(&argc, &argv);

	// Создаем главное окно, устанавливваем заголовок
	GtkWidget *widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(widget), "Clock");
	gtk_window_set_default_size (GTK_WINDOW(widget),800,250);

	// рамка
	gtk_container_set_border_width (GTK_CONTAINER (widget), 20);

	// создаем окно для цифр
	GtkWidget *area = gtk_drawing_area_new();
	g_signal_connect (area, "draw", G_CALLBACK (draw_area), 0);
	gtk_widget_set_size_request (area, 200, 100);
	gtk_container_add (GTK_CONTAINER (widget), area);

	// таймаут
	g_timeout_add(1000, &on_timeout, widget);
	
	// Соединяем сигналы
	g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(on_destroy), 0);
    g_signal_connect (widget, "key-press-event", G_CALLBACK (on_key_press_event), 0);

	// показать окно
	gtk_widget_show_all (widget);
	
	// бесконечный цикл обработки событий
	gtk_main();
	
	return 0;
}

gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event)
{
    if(event->keyval == 'q')
    {
        gtk_main_quit();
    }
    return true;
}

void on_destroy(GtkWidget *widget, gpointer user_data)
{
    gtk_main_quit();
}

gboolean on_timeout(gpointer user_data)
{
	GtkWidget *widget = (GtkWidget*) user_data;
	gtk_widget_queue_draw(widget);
    return true;
}

gboolean draw_area(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	// значения цифр
	unsigned int HH, H, MM, M, SS, S;
	time_t ct = time(NULL);
    struct tm *t = localtime(&ct);
    HH = t->tm_hour/10;
    H = t->tm_hour%10;
    MM = t->tm_min/10;
    M = t->tm_min%10;
    SS = t->tm_sec/10;
    S = t->tm_sec%10;
    
    // "привычный" контекст
	CairoContext cc(cr);

	// размер окна (виджета)
    GtkAllocation alc;
    gtk_widget_get_allocation(widget, &alc);
    int16_t x = alc.width, y = alc.height;
    
    // размер поля одной цифры
    int16_t x1 = x/7.75, y1 = y;
    Rect r(x1,y1);
    
    // десятки часов
    cc.Save();
    cc.SetMask(Point(0,0),r);
    draw_digit7(HH, &cc, r);
    cc.Restore();

    // единицы часов
    cc.Save();
    cc.SetMask(Point(1.25*x1,0),r);
    draw_digit7(H, &cc, r);
    cc.Restore();

    // десятки минут
    cc.Save();
    cc.SetMask(Point(2.75*x1,0),r);
    draw_digit7(MM, &cc, r);
    cc.Restore();

    // единицы минут
    cc.Save();
    cc.SetMask(Point(4*x1,0),r);
    draw_digit7(M, &cc, r);
    cc.Restore();

    // десятки секунд
    cc.Save();
    cc.SetMask(Point(5.5*x1,0),r);
    draw_digit7(SS, &cc, r);
    cc.Restore();

    // единицы секунд
    cc.Save();
    cc.SetMask(Point(6.75*x1,0),r);
    draw_digit7(S, &cc, r);
    cc.Restore();

    return true;
}

void draw_digit7(unsigned int digit, Context *cr, Rect &r)
{
    int16_t x = r.GetWidth(), y = r.GetHeight();
    int16_t s = x < y ? x : y;
    Point pts[6];
    double b = 0.5*0.1;//m_thickness;
    double f = sqrt(2.0)*0.5*0.01;//m_gap;

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
        if(data[digit][i])
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

            cr->SetColor(RGB(0,0,0.6)/*m_Color*/);
            cr->FillPolyline(6, pts);
        }
    }
}
