class Image : public Window
{
public:
    Image(const IMAGEINFO image=nullptr);
    ~Image();

    void OnDraw(Context *cr);

    void SetImage(const IMAGEINFO image);
    IMAGEINFO GetImage();
    void SetScale(double scaleX, double scaleY);
    void GetScale(double *scaleX, double *scaleY);
    void SetStyle(uint8_t style);
    uint8_t GetStyle();

private:
    IMAGEINFO m_image;
    double    m_scaleX, m_scaleY;
    uint8_t   m_style;
};
