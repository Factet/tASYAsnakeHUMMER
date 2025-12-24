#define FONTFACE_SIZE       100

#define TEXT_ALIGNH_MASK (TEXT_ALIGNH_LEFT|TEXT_ALIGNH_CENTER|TEXT_ALIGNH_RIGHT)
#define TEXT_ALIGNV_MASK (TEXT_ALIGNV_TOP|TEXT_ALIGNV_CENTER|TEXT_ALIGNV_BOTTOM)

class Text : public Window
{
public:
    Text(const char *text=nullptr);
    ~Text();

    void OnDraw(Context *cr);
    void OnSizeChanged();

    RGB  GetTextColor();
    void SetTextColor(const RGB textColor);

    void     GetFont(char **FontFace, uint16_t *FontSize, uint16_t *bold, uint16_t *italic);
    void     SetFont(const char *FontFace, const int16_t FontSize, const int16_t bold, const int16_t italic);
    uint8_t  GetAlignment();
    void     SetAlignment(uint8_t mode);
    bool     GetWrap();
    void     SetWrap(bool bWrap);
    Rect     &GetDataRect();                      // возвращает размер области в пикселях, занимаемой текстом
    uint16_t GetGap() { return m_adv/5; }

    bool     SetText(const char *text);
    char     *GetText();

private:
    uint8_t GetUnicodeSymbolSize(uint8_t firstbyte);    // определение размера символа UTF-8 по первому байту
    bool    CheckUTF8(const char *s);                   // проверка, состоит ли строка из допустимых символов UTF-8
    void    PrepareLines(Context *cr, uint16_t limit);  // подготовка текста: разбиение по строкам
    void    ClearLines();
    void    ComputeDataRect(const uint16_t width, const uint16_t limit); // вычисление размера данных

private:
    char *m_text;
    uint32_t m_textSize;
    RGB  m_textColor;
    uint16_t m_fontSize;
    int16_t m_ascent, m_descent;                // протяженность символов вверх и вниз от базовой линии
    uint16_t m_ls, m_adv;                       // межстрочный интервал(включая строку) и максимальная ширина символа
    char m_fontFace[FONTFACE_SIZE+1];
    uint8_t m_style;                            // TEXT_STYLE_BOLD, TEXT_STYLE_ITALIC
    bool m_bWrap;                               // перенос по словам
    bool m_bTextIsReady;                        // текст готов к отображению: разбит по строкам
    uint16_t m_nlines;                          // количество строк
    char **m_lines;                             // массив указателей на строки
    Rect m_textDimensions;                      // размер области с текстом
    Rect m_BackgroundSize;                      // размер текста для отрисовки фона
};
