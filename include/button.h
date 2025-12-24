class Button : public Window
{
public:
    Button(const uint32_t command);
    ~Button();

    void OnDraw(Context *cr);
    bool OnLeftMouseButtonClick(const Point &position);

    RGB  GetLiteColor();
    void SetLiteColor(const RGB textColor);
    RGB  GetDarkColor();
    void SetDarkColor(const RGB textColor);

    void SetCommand(const uint32_t command);

    void SetToggle(bool bToggle);
    bool GetToggle();
    void SetToggleState(bool bToggleState);
    bool GetToggleState();

private:
    uint32_t    m_command;

    RGB  m_liteColor;
    RGB  m_darkColor;
    bool m_bToggle, m_bToggleState;
};

class TextButton : public Button
{
public:
    TextButton(const char *text, const uint32_t command);
    ~TextButton();

    void OnCreate();

    RGB  GetTextColor();
    void SetTextColor(const RGB textColor);

    uint16_t GetFontSize();
    void     SetFontSize(const uint16_t textSize);

    void SetBackColor(const RGB backColor);

private:
    Text *m_pText;
    char m_text[100];

    RGB  m_textColor;
    uint16_t m_fontSize;
    char m_fontFace[100];   // add Get/Set
};

class ImageButton : public Button
{
public:
    ImageButton(const IMAGEINFO ii, const uint32_t command);
    ~ImageButton();

    void OnCreate();

    void SetStyle(uint8_t style);
    uint8_t GetStyle();
    void SetBackColor(const RGB backColor);

private:
    Image *m_pImage;
};
