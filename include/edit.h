#define TEXT_CHUNK_SIZE     100
#define FONTFACE_SIZE       100
#define POINTER_TIMEOUT     500

class Edit : public Window
{
public:
    Edit(const char *text=NULL);
    ~Edit();

    void OnDraw(Context *cr);
    bool OnLeftMouseButtonClick(const Point &position);

    RGB  GetTextColor();
    void SetTextColor(const RGB textColor);

    void     GetFont(char **FontFace, uint16_t *FontSize, uint16_t *bold, uint16_t *italic);
    void     SetFont(const char *FontFace, const int16_t FontSize, const int16_t bold, const int16_t italic);

    bool     SetText(const char *text);
    char     *GetText();
    uint32_t GetTextLength();                           // количество символов Unicode
    uint32_t GetTextBytes() { return m_textSize; }      // количество байтов

    bool OnKeyPress(uint64_t value);
    bool OnKeyboardCapture(bool bCapture);
    bool OnTimeout();

private:
    void ExpandText(uint32_t newsize);
    void SetPointer(uint32_t newPosition);              // установка указателя при помощи стрелок, Home, End
    void DrawPointer(Context *cr, const Point &ws);     // отрисовка указателя
    void Insert(uint64_t code, uint8_t s);
    void Delete();
    uint32_t GetByteIndex(uint32_t position);
    void CopyUnicodeSymbol(uint32_t &position);         // копирование одного символа UTF-8 из m_text в m_shadow
    uint8_t GetUnicodeSymbolSize(uint8_t firstbyte);    // определение размера символа UTF-8 по первому байту
    bool CheckUTF8(const char *s);                       // проверка, состоит ли строка из допустимых символов UTF-8

private:
    char *m_text, *m_shadow;                    // теневая копия содержит только символы до указателя (курсора)
    uint32_t m_textSize, m_maxtextSize, m_pointerPosition;
    RGB  m_activeColor, m_saveColor, m_textColor;
    uint16_t m_fontSize;
    int16_t m_ascent, m_descent;                // протяженность символов вверх и вниз от базовой линии
    uint16_t m_ls, m_adv;                       // межстрочный интервал(включая строку) и максимальная ширина символа
    char m_fontFace[FONTFACE_SIZE+1];
    bool m_bFocus;                              // окно обладает фокусом ввода
    uint8_t m_Pointer;                          // указатель текущего положения ввода отображается
    Point m_StoredClick;                        // запомненная точка щелчка ЛКМ
    bool  m_bStoredClick;
    uint8_t m_style;                            // TEXT_STYLE_BOLD, TEXT_STYLE_ITALIC
};
