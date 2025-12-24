// context.h

// стили текста
#define TEXT_STYLE_BOLD         0x01
#define TEXT_STYLE_ITALIC       0x02

#define TEXT_ALIGNH_LEFT        0x04
#define TEXT_ALIGNH_RIGHT       0x08
#define TEXT_ALIGNH_CENTER      0x10

#define TEXT_ALIGNV_TOP         0x20
#define TEXT_ALIGNV_BOTTOM      0x40
#define TEXT_ALIGNV_CENTER      0x80


// стили картинок
#define IMAGE_SCALE_1_1         0x00    // картинка отображается в масштабе 1:1, как поместилась
#define IMAGE_SCALE_FIT         0x01    // картика масштабируется, чтобы поместиться в поле, соотношение сторон сохраняется
#define IMAGE_SCALE_STRETCH     0x02    // картика масштабируется, занимает все поле, соотношение сторон м.б. искажено
#define IMAGE_SCALE_XY          0x03    // картинка масштабируется по X и Y, как поместилась

#define IMAGE_SCALE_MASK        0x03

#define IMAGE_ALIGNH_LEFT       0x04
#define IMAGE_ALIGNH_RIGHT      0x08
#define IMAGE_ALIGNH_CENTER     0x10

#define IMAGE_ALIGNV_TOP        0x20
#define IMAGE_ALIGNV_BOTTOM     0x40
#define IMAGE_ALIGNV_CENTER     0x80


class Context
{
public:
    Context() {}
    virtual ~Context() {}

    virtual void Save() = 0;
    virtual void Restore() = 0;
    virtual void SetMask(const Point &pos, const Rect &size) = 0;
    virtual void SetPosition(const Point &pos) = 0;
    virtual void SetOrigin(const Point &pos) = 0;

    virtual void SetColor(const RGB &clr) = 0;
    virtual void SetLineWidth(const uint16_t width) = 0;

    virtual void FillRectangle(const Point &from, const Point &to) = 0;
    virtual void FillRectangle(const Point &from, const Rect &size) = 0;
    virtual void Rectangle(const Point &from, const Point &to) = 0;
    virtual void Rectangle(const Point &from, const Rect &size) = 0;
    virtual void Line(const Point &from, const Point &to) = 0;
    virtual void Text(const char *text, const char *fontface,
        const uint16_t fontsize, const Point &pt, const uint32_t style, uint16_t *advance=0) = 0;
    virtual void GetTextInfo(const char *text, const char *fontface,
        const uint16_t fontsize, const uint32_t style, uint16_t *width,
        uint16_t *height, uint16_t *advance) = 0;
    virtual void GetFontInfo(const char *fontface, const uint16_t fontsize, const uint32_t style,
        int16_t *ascent, int16_t *descent, uint16_t *linespacing, uint16_t *maxadvance) = 0;
    virtual void Polyline(const uint16_t n, const Point p[]) = 0;
    virtual void FillPolyline(const uint16_t n, const Point p[]) = 0;
    virtual IMAGEINFO LoadPNG(const char *filename) = 0;
    virtual void DeletePNG(IMAGEINFO ii) = 0;
    virtual void Image(const IMAGEINFO ii, const Point &position, double scaleX, double scaleY) = 0;
};

