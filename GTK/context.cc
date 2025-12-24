// context.cc

#include <iostream>
#include <gtk/gtk.h>
#include "mytypes.h"
#include "context.h"

CairoContext::CairoContext(cairo_t *cr)
{
    SetCairoContext(cr);
    m_width = 1;
}

CairoContext::~CairoContext()
{
}

void CairoContext::SetCairoContext(cairo_t *cr)
{
    m_cr = cr;
    cairo_scale(m_cr, 1.0, 1.0);
}

void CairoContext::Save()
{
    cairo_save(m_cr);
}

void CairoContext::Restore()
{
    cairo_restore(m_cr);
}

void CairoContext::SetMask(const Point &pos, const Rect &size)
{
    cairo_translate(m_cr, pos.GetX(), pos.GetY());
    cairo_rectangle(m_cr, 0, 0, size.GetWidth(), size.GetHeight());
    cairo_clip(m_cr);
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
	cairo_rectangle(m_cr, from.GetX(), from.GetY(), rectsize.GetX(), rectsize.GetY());
    cairo_fill(m_cr);
}

void CairoContext::FillRectangle(const Point &from, const Rect &rectsize)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
	cairo_rectangle(m_cr, from.GetX(), from.GetY(), rectsize.GetWidth(), rectsize.GetHeight());
    cairo_fill(m_cr);
}

void CairoContext::Rectangle(const Point &from, const Point &rectsize)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
	double d = m_width&1 ? 0.5 : 0;
	cairo_rectangle(m_cr, d+from.GetX(), d+from.GetY(), rectsize.GetX(), rectsize.GetY());
    cairo_stroke(m_cr);
}

void CairoContext::Rectangle(const Point &from, const Rect &rectsize)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
	double d = m_width&1 ? 0.5 : 0;
	cairo_rectangle(m_cr, d+from.GetX(), d+from.GetY(), rectsize.GetWidth(), rectsize.GetHeight());
    cairo_stroke(m_cr);
}

void CairoContext::Line(const Point &from, const Point &to)
{
	cairo_set_source_rgba(m_cr, m_color.GetRed(), m_color.GetGreen(), m_color.GetBlue(), 1.0);
	cairo_set_line_width (m_cr, m_width);
	double d = m_width&1 ? 0.5 : 0;
    cairo_move_to(m_cr, d+from.GetX(), d+from.GetY());
    cairo_line_to(m_cr, d+to.GetX(), d+to.GetY());
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

    cairo_move_to (m_cr, x, y);
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
        double d = m_width&1 ? 0.5 : 0;
        cairo_move_to(m_cr, d+p[0].GetX(), d+p[0].GetY());
        for(uint16_t i=1; i<n; i++)
        {
            cairo_line_to(m_cr, d+p[i].GetX(), d+p[i].GetY());
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
        cairo_move_to(m_cr, p[0].GetX(), p[0].GetY());
        for(uint16_t i=1; i<n; i++)
        {
            cairo_line_to(m_cr, p[i].GetX(), p[i].GetY());
        }
        cairo_fill(m_cr);
    }
}

void CairoContext::DisplayImage(const IMAGEINFO ii, const Point &position, double scaleX, double scaleY)
{
    cairo_surface_t *image = (cairo_surface_t *) ii->imageptr;

    cairo_save(m_cr);
    cairo_scale (m_cr, scaleX, scaleY);

    cairo_set_source_surface (m_cr, image, (position.GetX())/scaleX, (position.GetY())/scaleY);
    cairo_paint (m_cr);
    cairo_restore(m_cr);
}


