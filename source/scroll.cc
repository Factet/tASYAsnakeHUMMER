#include <cassert>
#include "window.h"
#include "scroll.h"

Scroll::Scroll()
{
    m_ClassName = __FUNCTION__;
    SetFrameWidth(0);
    m_pDataWindow = nullptr;
    m_pHBar = new HorizontalScrollBar;
    m_pVBar = new VerticalScrollBar;

    m_dx = 0;
    m_dy = 0;
    m_saveOrigin = Point(0,0);
}

Scroll::~Scroll()
{
}

void Scroll::SetBackColor(const RGB backColor)
{
    Window::SetBackColor(m_backColor);
    m_pHBar->SetBackColor(m_backColor);
    m_pVBar->SetBackColor(m_backColor);
}

Rect &Scroll::GetInteriorSize()
{
    m_InteriorSize = GetSize() - Rect(2*GetFrameWidth()+SCROLLBAR_SIZE, 2*GetFrameWidth()+SCROLLBAR_SIZE);
    return m_InteriorSize;
}

void Scroll::OnSizeChanged()
{
    Point is = GetInteriorSize();

    // горизонтальная шкала прокрутки
    m_pHBar->SetSize(Rect(is.GetX(),SCROLLBAR_SIZE));
    m_pHBar->SetPosition(Point(0,is.GetY()));
    m_pHBar->Update();

    // вертикальная шкала прокрутки
    m_pVBar->SetSize(Rect(SCROLLBAR_SIZE,is.GetY()));
    m_pVBar->SetPosition(Point(is.GetX(),0));
    m_pVBar->Update();

    // окно с данными (с прокруткой)
    m_pDataWindow->SetSize(Rect(is.GetX(), is.GetY()));
    m_pDataWindow->SetPosition(Point(0,0));

    ReDraw();
}

void Scroll::OnDraw(Context *cr)
{
    Point is = GetInteriorSize();

    // фон
    cr->SetColor(m_backColor);
    cr->FillRectangle(is,Point(is.GetX()+SCROLLBAR_SIZE,is.GetY()+SCROLLBAR_SIZE));

    // размер данных для прокрутки
    Rect d = m_pDataWindow->GetDataRect();
    Point p = m_pDataWindow->GetOrigin();
    m_saveOrigin = p;
    m_pHBar->SetTotal(d.GetWidth());
    m_pHBar->SetDataOrigin(p.GetX());
    m_pVBar->SetTotal(d.GetHeight());
    m_pVBar->SetDataOrigin(p.GetY());
}

void Scroll::OnCreate()
{
    // создание шкал прокрутки

    Rect is = GetInteriorSize();

    // шкалы прокрутки
    AddChild(m_pHBar,Point(0,is.GetHeight()),Rect(is.GetWidth(),SCROLLBAR_SIZE));
    m_pHBar->SetDataOrigin(0);
    AddChild(m_pVBar,Point(is.GetWidth(),0),Rect(SCROLLBAR_SIZE,is.GetHeight()));
    m_pVBar->SetDataOrigin(0);
}

void Scroll::OnNotify(Window *child, uint32_t type, const Point &position)
{
    if((child == m_pHBar || child == m_pVBar) && (type == EVENT_SCROLLING))
    {
        // обработка событий скроллбаров

        uint32_t x = m_pHBar->GetDataOrigin();
        uint32_t y = m_pVBar->GetDataOrigin();

        m_saveOrigin = Point(x,y);
        m_pDataWindow->SetOrigin(m_saveOrigin);
        m_pDataWindow->ReDraw();
    }
    else if(child == m_pDataWindow)
    {
        // обо всех прочих событиях уведомляем родителя
        NotifyParent(type, position);
    }
}

bool Scroll::OnScroll(uint64_t value)
{
    SCROLLINFO si = (SCROLLINFO) value;

    switch(si->direction)
    {
    case _SCROLLINFO::SCROLL_LEFT:
        m_dx -= SCROLL_DELTA;
        break;
    case _SCROLLINFO::SCROLL_RIGHT:
        m_dx += SCROLL_DELTA;
        break;
    case _SCROLLINFO::SCROLL_UP:
        m_dy -= SCROLL_DELTA;
        break;
    case _SCROLLINFO::SCROLL_DOWN:
        m_dy += SCROLL_DELTA;
        break;
    case _SCROLLINFO::SCROLL_SMOOTH:
        m_dx -= si->dx;
        m_dy -= si->dy;
        break;
    }

    // текущая прокрутка
    Point origin = m_saveOrigin;
    uint32_t x, y;

    x = m_pHBar->SetDataOrigin(origin.GetX() + m_dx);
    y = m_pVBar->SetDataOrigin(origin.GetY() + m_dy);

    origin.SetX(x);
    origin.SetY(y);
    m_pDataWindow->SetOrigin(origin);
    m_pDataWindow->ReDraw();

    // если прокрутка закончена
    if(si->stop)
    {
        m_dx = 0;
        m_dy = 0;
        m_saveOrigin = m_pDataWindow->GetOrigin();
    }

    return true;
}

// задание окна, для которого будут отображаться шкалы прокрутки
void Scroll::SetDataWindow(Window *pWindow)
{
    assert(m_pDataWindow == nullptr);

    // запомним окно с данными
    m_pDataWindow = pWindow;

    Rect is = GetInteriorSize();

    // окно с данными
    AddChild(m_pDataWindow, Point(0,0), is);
}

// уведомление от окна с данными об изменениии размера
void Scroll::OnDataRectChanged(const Window *pWindow, const Rect &d)
{
    if(pWindow != m_pDataWindow)
    {
        return;
    }

    Point p = m_pDataWindow->GetOrigin();
    m_pHBar->SetTotal(d.GetWidth());
    m_pHBar->SetDataOrigin(p.GetX());
    m_pHBar->Update();
    m_pVBar->SetTotal(d.GetHeight());
    m_pVBar->SetDataOrigin(p.GetY());
    m_pVBar->Update();
    m_pDataWindow->SetOrigin(Point(m_pHBar->GetDataOrigin(),m_pVBar->GetDataOrigin()));
    ReDraw();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////

ScrollBar::ScrollBar()
{
    m_ClassName = __FUNCTION__;
    m_scrollerColor = RGB(0.4, 0.4, 0.4);
    SetFrameWidth(0);
    m_bDrag = false;
}

ScrollBar::~ScrollBar()
{
}

RGB ScrollBar::GetScrollerColor()
{
    return m_scrollerColor;
}

void ScrollBar::SetScrollerColor(const RGB Color)
{
    m_scrollerColor = Color;
}

void ScrollBar::SetTotal(uint32_t total)
{
    m_total = total;
}

uint32_t ScrollBar::GetDataOrigin()
{
    return m_DataOrigin;
}

uint32_t ScrollBar::SetDataOrigin(int32_t origin)
{
    uint16_t q = GetScrollbarSize(GetInteriorSize());
    if(origin < 0)
    {
        m_DataOrigin = 0;
    }
    else if(origin + q > m_total)
    {
        if(m_total > q)
        {
            m_DataOrigin = m_total - q;
        }
        else
        {
            m_DataOrigin = 0;
        }
    }
    else
    {
        m_DataOrigin = origin;
    }

    return m_DataOrigin;
}

// обработка щелчков ЛКМ в скролбаре
bool ScrollBar::OnLeftMouseButtonClick(const Point &position)
{
    // размер окна
    uint16_t q = GetScrollbarSize(GetInteriorSize());

    // позиция щелчка
    uint16_t pos = GetPos(position);

    // как позиция щелчка соотносится с положением скроллера?
    if(pos < m_p)
    {
        // левее
        m_DataOrigin = m_DataOrigin > q ? m_DataOrigin-q : 0;
    }
    else if (pos > m_p+m_s)
    {
        // правее
        m_DataOrigin = m_DataOrigin+q < m_total-q ? m_DataOrigin+q : m_total-q;
    }

    // щелчок в скроллере - начало буксировки
    if(pos >= m_p && pos <= m_p+m_s && !m_bDrag)
    {
        m_bDrag = true;
        m_nDragStart = pos;
        CaptureMouse(this);
    }

    // уведомим родительское окно
    NotifyParent(EVENT_SCROLLING, Point(0,0));
    return true;
}

bool ScrollBar::OnMouseMove(const Point &position)
{
    if(m_bDrag)
    {
        // размер окна
        uint16_t q = GetScrollbarSize(GetInteriorSize());

        // текущая позиция (корректируем, если меньше 0)
        uint16_t pos = GetPos(position)&(1<<(sizeof(pos)*8-1)) ? 0 : GetPos(position);

        // коэффициент
        double c = ((double)m_total)/q;

        if(pos < m_nDragStart)
        {
            m_DataOrigin = m_DataOrigin > (m_nDragStart-pos)*c ? m_DataOrigin-(m_nDragStart-pos)*c : 0;
        }
        else
        {
            m_DataOrigin = m_DataOrigin+(pos-m_nDragStart)*c < m_total-q ? m_DataOrigin+(pos-m_nDragStart)*c : m_total-q;
        }

        // уведомим родительское окно
        NotifyParent(EVENT_SCROLLING, Point(0,0));

        m_nDragStart = pos;
    }

    return m_bDrag;
}

bool ScrollBar::OnLeftMouseButtonRelease(const Point &position)
{
    m_bDrag = false;
    CaptureMouse(nullptr);
    return true;
}

void ScrollBar::Update()
{
    uint16_t q = GetScrollbarSize(GetInteriorSize());

    if(m_DataOrigin+q > m_total)
    {
        if(m_total > q)
        {
            m_DataOrigin = m_total - q;
        }
        else
        {
            m_DataOrigin = 0;
        }

        // уведомим родительское окно
        NotifyParent(EVENT_SCROLLING, Point(0,0));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

HorizontalScrollBar::HorizontalScrollBar()
{
    m_ClassName = __FUNCTION__;
}

HorizontalScrollBar::~HorizontalScrollBar()
{
}

void HorizontalScrollBar::OnDraw(Context *cr)
{
    Point p1, p2;
    Rect is = GetInteriorSize();

    // рисуем скроллбар
    p1 = Point(0,0);
    p2 = Point(is.GetWidth(),is.GetHeight());
    cr->SetColor(m_backColor);
    cr->FillRectangle(p1,p2);

    uint16_t t = GetScrollbarSize(is);
    if(t < m_total)
    {
        // размер скроллера
        m_s = max((uint16_t) (((double)t)*t/m_total),MIN_SCROLLER_SIZE);

        // положение скроллера
        m_p = m_DataOrigin * (t-m_s) / (m_total-t);

        // рисуем скроллер
        p1 = Point(m_p,0);
        p2 = Point(m_s,is.GetHeight());
        cr->SetColor(m_scrollerColor);
        cr->FillRectangle(p1,p2);
    }
    else
    {
        m_s = t;
        m_p = 0;
    }
}

uint16_t HorizontalScrollBar::GetScrollbarSize(const Rect &r)
{
    return r.GetWidth();
}

uint16_t HorizontalScrollBar::GetPos(const Point &p)
{
    return p.GetX();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

VerticalScrollBar::VerticalScrollBar()
{
    m_ClassName = __FUNCTION__;
}

VerticalScrollBar::~VerticalScrollBar()
{
}

void VerticalScrollBar::OnDraw(Context *cr)
{
    Point p1, p2;
    Rect is = GetInteriorSize();

    // рисуем скроллбар
    p1 = Point(0,0);
    p2 = Point(is.GetWidth(),is.GetHeight());
    cr->SetColor(m_backColor);
    cr->FillRectangle(p1,p2);

    uint16_t t = GetScrollbarSize(is);
    if(t < m_total)
    {
        // размер скроллера
        m_s = max((uint16_t) (((double)t)*t/m_total),MIN_SCROLLER_SIZE);

        // положение скроллера
        m_p = m_DataOrigin * (t-m_s) / (m_total-t);

        // рисуем скроллер
        p1 = Point(0,m_p);
        p2 = Point(is.GetWidth(),m_s);
        cr->SetColor(m_scrollerColor);
        cr->FillRectangle(p1,p2);

    }
    else
    {
        m_s = t;
        m_p = 0;
    }
}


uint16_t VerticalScrollBar::GetScrollbarSize(const Rect &r)
{
    return r.GetHeight();
}

uint16_t VerticalScrollBar::GetPos(const Point &p)
{
    return p.GetY();
}
