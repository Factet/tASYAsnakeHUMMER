#include "window.h"
#include "image.h"

Image::Image(const IMAGEINFO image)
{
    m_ClassName = __FUNCTION__;
    SetFrameWidth(0);
    SetImage(image);
    m_scaleX = 1.0;
    m_scaleY = 1.0;
    m_style = IMAGE_SCALE_1_1 | IMAGE_ALIGNH_LEFT |IMAGE_ALIGNV_TOP;
}

Image::~Image()
{
}

void Image::SetImage(const IMAGEINFO image)
{
    m_image = image;
}

IMAGEINFO Image::GetImage()
{
    return m_image;
}

void Image::OnDraw(Context *cr)
{
    Point ws = GetInteriorSize();

    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(0,0),ws);

    if(m_image)
    {
        double sx, sy;      // масштаб по X и Y
        uint16_t gx, gy;    // отступ от ЛВУ по X и Y

        if((m_style & IMAGE_SCALE_MASK) == IMAGE_SCALE_FIT)
        {
            double rw = ((double)ws.GetX())/((double)ws.GetY());
            double ri = ((double)m_image->width)/((double)m_image->height);
            if(rw>ri)
            {
                sx = ((double)ws.GetY())/((double)m_image->height);
                sy = sx;
                gx = ws.GetX()-sx*m_image->width;
                gy = 0;
            }
            else
            {
                sx = ((double)ws.GetX())/((double)m_image->width);
                sy = sx;
                gx = 0;
                gy = ws.GetY()-sx*m_image->height;
            }
        }
        else if((m_style & IMAGE_SCALE_MASK) == IMAGE_SCALE_STRETCH)
        {
            sx = ((double)ws.GetX())/m_image->width;
            sy = ((double)ws.GetY())/m_image->height;
            gx = 0;
            gy = 0;
        }
        else if((m_style & IMAGE_SCALE_MASK) == IMAGE_SCALE_XY)
        {
            sx = m_scaleX;
            sy = m_scaleY;
            gx = ws.GetX() > m_image->width*sx ? ws.GetX() - m_image->width*sx : 0;
            gy = ws.GetY() > m_image->height*sy ? ws.GetY() - m_image->height*sy : 0;
        }
        else
        {
            // m_style & IMAGE_SCALE_MASK == IMAGE_SCALE1_1_1
            sx = 1.0;
            sy = 1.0;
            gx = ws.GetX() > m_image->width ? ws.GetX() - m_image->width : 0;
            gy = ws.GetY() > m_image->height ? ws.GetY() - m_image->height : 0;
        }

        // положение по горизонтали
        if(gx > 0)
        {
            if(m_style & IMAGE_ALIGNH_LEFT)
            {
                gx = 0;
            }
            else if(m_style & IMAGE_ALIGNH_RIGHT)
            {
                // gx готово
            }
            else // по центру
            {
                gx = gx/2;
            }
        }

        // положение по вертикали
        if(gy > 0)
        {
            if(m_style & IMAGE_ALIGNV_TOP)
            {
                gy = 0;
            }
            else if(m_style & IMAGE_ALIGNV_BOTTOM)
            {
                // gy готово
            }
            else // по центру
            {
                gy = gy/2;
            }
        }

        cr->Image(m_image, Point(gx,gy), sx, sy);
    }
}

void Image::SetScale(double scaleX, double scaleY)
{
    m_scaleX = scaleX;
    m_scaleY = scaleY;
}

void Image::GetScale(double *scaleX, double *scaleY)
{
    if(scaleX)
    {
        *scaleX = m_scaleX;
    }
    if(scaleY)
    {
        *scaleY = m_scaleY;
    }
}

void Image::SetStyle(uint8_t style)
{
    m_style = style;
}

uint8_t Image::GetStyle()
{
    return m_style;
}
