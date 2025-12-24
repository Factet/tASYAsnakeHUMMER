class Digit7 : public Window
{
public:
    Digit7(uint8_t d=0);
    ~Digit7();

    void OnDraw(Context *cr);

    RGB  GetColor();
    void SetColor(RGB Color);
    double   GetGap();
    void     SetGap(double gap);
    double   GetThickness();
    void     SetThickness(double thickness);

    uint8_t  GetDigit();
    void     SetDigit(uint8_t d);

private:
    uint8_t m_digit;
    RGB  m_Color;
    double m_gap;
    double m_thickness;
};
