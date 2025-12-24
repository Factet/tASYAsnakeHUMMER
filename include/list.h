// class List

// оповещения родителя от списка
enum ListEventTypes
{
    EVENT_LIST_LMB_CLICK = 1,
    EVENT_LIST_LMB_DOUBLECLICK,
    EVENT_LIST_RMB_CLICK,
    EVENT_LIST_RMB_DOUBLECLICK,
};

class List : public Window
{
public:
    List();
    ~List();

    uint16_t GetNumberOfElements() const;
    void     Clear();
    Window   *GetElement(const uint16_t n) const;
    void     *GetValue(const uint16_t n) const;
    void     Insert(const uint16_t position, Window *pElement, void *value=nullptr);
    void     Delete(const uint16_t n);

    bool     OnLeftMouseButtonClick(const Point &position);
    bool     OnLeftMouseButtonDoubleClick(const Point &position);
    void     OnDraw(Context *cr);
    void     OnSizeChanged();
    Rect     &GetDataRect();
    void     OnDataRectChanged(const Window *pWindow, const Rect &rect);

    uint16_t GetElementHeight() const;
    void     SetElementHeight(const uint16_t height);
    int16_t  GetSelection() const;
    void     SetSelection(const int16_t n);
    RGB      GetSelBackColor() const;
    void     SetSelBackColor(const RGB Color);

private:

    void     RepositionElements();

    uint16_t m_nElements;
    Window   **m_pElements;
    void     **m_pValues;
    uint16_t m_ElementHeight;
    uint16_t m_maxWidth;
    Rect     m_DataRect;
    int16_t  m_nSelection;
    RGB      m_selbackColor;
    Rect     m_BackgroundSize;
};

