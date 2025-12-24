#define SCROLLBAR_SIZE      10
#define MIN_SCROLLER_SIZE   50

#define SCROLL_DELTA        24.0

class HorizontalScrollBar;
class VerticalScrollBar;

class Scroll : public Window
{
public:
    Scroll();
    ~Scroll();

    void OnDraw(Context *cr);
    void OnSizeChanged();
    void OnCreate();
    void OnNotify(Window *child, uint32_t type, const Point &position);
    bool OnScroll(uint64_t value);
    void OnDataRectChanged(const Window *pWindow, const Rect &rect);

    Rect &GetInteriorSize();

    void SetBackColor(const RGB backColor);

    void SetDataWindow(Window *pWindow);        // задание окна, для которого будут отображаться шкалы прокрутки

private:
    Window *m_pDataWindow;
    HorizontalScrollBar *m_pHBar;
    VerticalScrollBar   *m_pVBar;
    double m_dx, m_dy;                          // текущее смещение скроллинга (обнуляется при окончании)
    Point m_saveOrigin;                         // положение окна в документе до начала скролинга
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

// типы событий
enum ScrollBarEventType
{
    EVENT_SCROLLING = 1,
};

class ScrollBar : public Window
{
public:
    ScrollBar();
    ~ScrollBar();

    void SetTotal(uint32_t total);                  // устанавливает общий размер данных (документ) в пикселях
    uint32_t GetDataOrigin();                       // возвращает положение окна в документе в пикселях
    uint32_t SetDataOrigin(int32_t origin);          // устанавливает положение окна в документе в пикселях
    void Update();                                  // обновление при изменении размера
    bool OnLeftMouseButtonClick(const Point &position);
    bool OnMouseMove(const Point &position);
    bool OnLeftMouseButtonRelease(const Point &position);

    virtual uint16_t GetScrollbarSize(const Rect &r)=0; // возвращает размер скроллбара, для гориз. - ширину, для верт. - высоту
    virtual uint16_t GetPos(const Point &p)=0;          // возвращает позицию щелчка, для гориз. - X, для верт. - Y

    RGB  GetScrollerColor();
    void SetScrollerColor(const RGB textColor);

protected:
    uint32_t m_total, m_DataOrigin;
    RGB  m_scrollerColor;

    uint16_t m_p, m_s;                              // положение и размер скроллера
    bool m_bDrag;
    uint16_t m_nDragStart;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

class HorizontalScrollBar : public ScrollBar
{
public:
    HorizontalScrollBar();
    ~HorizontalScrollBar();

    void OnDraw(Context *cr);

    virtual uint16_t GetScrollbarSize(const Rect &r); // возвращает размер скроллбара, для гориз. - ширину, для верт. - высоту
    virtual uint16_t GetPos(const Point &p);          // возвращает позицию щелчка, для гориз. - X, для верт. - Y
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

class VerticalScrollBar : public ScrollBar
{
public:
    VerticalScrollBar();
    ~VerticalScrollBar();

    void OnDraw(Context *cr);

    virtual uint16_t GetScrollbarSize(const Rect &r); // возвращает размер скроллбара, для гориз. - ширину, для верт. - высоту
    virtual uint16_t GetPos(const Point &p);          // возвращает позицию щелчка, для гориз. - X, для верт. - Y
};

