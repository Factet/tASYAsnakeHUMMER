#include <cassert>
#include <iostream>
#include "window.h"

Window::Window()
{
    m_ClassName = __FUNCTION__;
    m_pParent = nullptr;
    m_pMyFirstChild = nullptr;
    m_pNextChild = nullptr;
    m_bToBeDeleted = false;
    m_bCreated = false;
    m_frameColor = RGB(0.0, 0.0, 0.0);
    m_backColor = RGB(0.9, 0.9, 0.9);
    m_frameWidth = 0;
    m_origin = Point(0,0);
    m_bShow = true;
}

Window::~Window()
{
}

bool Window::WindowProc(uint32_t type, const Point &pos, uint64_t value)
{
    bool result = false;

    Point position = pos + m_origin - Point(m_frameWidth, m_frameWidth);

    // событие - щелчок кнопки или перемещение мыши ?
    if(type == EVENT_LEFTMOUSEBUTTONCLICK || type == EVENT_RIGHTMOUSEBUTTONCLICK
        || type == EVENT_LEFTMOUSEBUTTONDOUBLECLICK || type == EVENT_RIGHTMOUSEBUTTONDOUBLECLICK
        || type == EVENT_LEFTMOUSEBUTTONRELEASE || type == EVENT_RIGHTMOUSEBUTTONRELEASE
        || type == EVENT_MOUSEMOVE || type == EVENT_SCROLL)
    {
        // если событие относится к дочернему окну, делегируем его обработку процедуре дочернего окна
        Window **pPrevNext = &m_pMyFirstChild;
        for(Window *pChild = m_pMyFirstChild; pChild; pChild = pChild->m_pNextChild)
        {
            Point from = pChild->GetPosition();
            Point to = from + pChild->GetSize();

            if((position.GetX() >= from.GetX()) && (position.GetX() <= to.GetX())
                && (position.GetY() >= from.GetY()) && (position.GetY() <= to.GetY())
                && pChild->m_bShow)
            {
                // вызываем процедуру дочернего окна
                result = pChild->WindowProc(type, position-from, value);

                // дочернее окно запросило удаление?
                if(pChild->m_bToBeDeleted)
                {
                    Window *next = pChild->m_pNextChild;
                    *pPrevNext = next;
                    Destroy(pChild);
                    ReDraw();
                }
                break;
            }

            pPrevNext = &pChild->m_pNextChild;
        }

        // если обработка завершена, уходим
        if(result)
        {
            return result;
        }

        // стандартные события
        switch(type)
        {
        case EVENT_LEFTMOUSEBUTTONCLICK:
            return OnLeftMouseButtonClick(position);
        case EVENT_RIGHTMOUSEBUTTONCLICK:
            return OnRightMouseButtonClick(position);
        case EVENT_LEFTMOUSEBUTTONDOUBLECLICK:
            return OnLeftMouseButtonDoubleClick(position);
        case EVENT_RIGHTMOUSEBUTTONDOUBLECLICK:
            return OnRightMouseButtonDoubleClick(position);
        case EVENT_LEFTMOUSEBUTTONRELEASE:
            return OnLeftMouseButtonRelease(position);
        case EVENT_RIGHTMOUSEBUTTONRELEASE:
            return OnRightMouseButtonRelease(position);
        case EVENT_MOUSEMOVE:
            return OnMouseMove(position);
        case EVENT_SCROLL:
            return OnScroll(value);
        default:
            return false;
        }
    }
    // событие - изменение размера окна ?
    else if(type == EVENT_WINDOWRESIZE)
    {
        Rect size(pos.GetX(), pos.GetY());
        SetSize(size);
        return true;
    }
    // событие - истечение интервала таймера ?
    else if(type == EVENT_TIMEOUT)
    {
        return OnTimeout();
    }
    // событие - нажатие клавиши ?
    else if(type == EVENT_KEYPRESS)
    {
        return OnKeyPress(value);
    }
    // событие - захват фокуса ввода ?
    else if(type == EVENT_KEYBOARDCAPTURE)
    {
        return OnKeyboardCapture(true);
    }
    // событие - освобождение фокуса ввода ?
    else if(type == EVENT_KEYBOARDRELEASE)
    {
        return OnKeyboardCapture(false);
    }
    // других событий мы не знаем
    else
    {
        return false;
    }
}

void Window::Draw(Context *cr)
{
    if(!m_bCreated || !m_bShow)
    {
        return;
    }

    cr->Save();

    // положение и размер окна
    Point position = Point(0,0);
    for(Window *p=this;p;p=p->GetParent())
    {
        position = position + p->GetPosition();
        if(p!=this)
        {
            uint16_t f = p->GetFrameWidth();
            position = position + Point(f,f);
        }
    }
    cr->SetPosition(position);
    Rect size = GetSize();

    // скроллинг
    cr->SetOrigin(m_origin);

    // рисуем во всем окне
    cr->SetMask(position, size);

    // если должна быть рамка
    if(m_frameWidth>0)
    {
        // рисуем рамку
        DrawFrame(cr);

        // маска для внутренней области окна
        position = position + Point(m_frameWidth, m_frameWidth);
        cr->SetPosition(position);
        size = GetInteriorSize();

        cr->SetMask(position, size);
    }

    // вызываем метод отрисовки содержимого
    OnDraw(cr);

    // отрисовка дочерних окон
    for(Window *pChild = m_pMyFirstChild; pChild; pChild = pChild->m_pNextChild)
    {
        pChild->Draw(cr);
    }

    cr->Restore();
}

void Window::OnDraw(Context *cr)
{
}

void Window::ReDraw()
{
    if(m_bCreated)
    {
        m_pParent->ReDraw();
    }
}

void Window::Create(Window *parent)
{
    m_pParent = parent;

    // вызываем виртуальный метод OnCreate() для создания дочерних окон классов-наследников
    OnCreate();

    m_bCreated = true;
    CaptureKeyboard(this);
}

void Window::OnCreate()
{
}

void Window::AddChild(Window *child, const Point &position, const Rect &size)
{
    child->m_pNextChild = m_pMyFirstChild;
    m_pMyFirstChild = child;

    // цвет фона дочернего окна такой же, как у родительского
    child->SetBackColor(GetBackColor());

    child->SetPosition(position);
    child->SetSize(size);
    child->Create(this);
}

void Window::DeleteChild(Window *pWin)
{
    Window **pPrevNext = &m_pMyFirstChild;
    for(Window *pChild = m_pMyFirstChild; pChild; pChild = pChild->m_pNextChild)
    {
        // найдено удаляемое окно?
        if(pChild == pWin)
        {
            Window *next = pChild->m_pNextChild;
            *pPrevNext = next;

            break;
        }
        pPrevNext = &pChild->m_pNextChild;
    }

    Destroy(pWin);
}

void Window::DeleteAllChildren()
{
    while(m_pMyFirstChild)
    {
        DeleteChild(m_pMyFirstChild);
    }
}

void Window::Destroy(Window *pChild)
{
    if(HasKeyboard(pChild))
    {
        CaptureKeyboard(0);
    }

    // удаление дочерних окон
	pChild->DeleteAllChildren();

    delete pChild;
}

void Window::NotifyParent(uint32_t type, const Point &position)
{
    assert(m_pParent);
    m_pParent->Notify(this, type, position);
}

void Window::Notify(Window *child, uint32_t type, const Point &position)
{
    // координаты точки в дочернем окне
    Point pos = child->GetPosition() + position;
    OnNotify(child, type, pos);
}

void Window::OnNotify(Window *child, uint32_t type, const Point &position)
{
}

Rect &Window::GetSize()
{
    return m_size;
}

Rect &Window::GetInteriorSize()
{
    return m_InteriorSize;
}

void Window::SetSize(const Rect &size)
{
    m_size = size;
    m_InteriorSize = size - Rect(2*m_frameWidth, 2*m_frameWidth);
    if(m_bCreated)
    {
        OnSizeChanged();
    }
}

void Window::OnSizeChanged()
{
}

Point &Window::GetPosition()
{
    return m_position;
}

void Window::SetPosition(const Point &position)
{
    m_position = position;
}

void Window::CreateTimeout(Window *pWindow, uint32_t timeout)
{
    m_pParent->CreateTimeout(pWindow, timeout);
}

void Window::CaptureKeyboard(Window *pWindow)
{
    m_pParent->CaptureKeyboard(pWindow);
}

void Window::CaptureMouse(Window *pWindow)
{
    m_pParent->CaptureMouse(pWindow);
}

bool Window::HasKeyboard(Window *pWindow)
{
    return m_pParent->HasKeyboard(pWindow);
}

RGB  Window::GetBackColor()
{
    return m_backColor;
}

void Window::SetBackColor(const RGB backColor)
{
    m_backColor = backColor;
}

RGB  Window::GetFrameColor()
{
    return m_frameColor;
}

void Window::SetFrameColor(const RGB frameColor)
{
    m_frameColor = frameColor;
}

uint16_t Window::GetFrameWidth()
{
    return m_frameWidth;
}

void     Window::SetFrameWidth(const uint16_t frameWidth)
{
    m_frameWidth = frameWidth;
    m_InteriorSize = m_size - Rect(2*m_frameWidth, 2*m_frameWidth);
}

void Window::DrawFrame(Context *cr)
{
    // нарисуем рамку
    if(m_frameWidth > 0)
    {
        Point ws = GetSize();

        cr->SetColor(m_frameColor);

        cr->FillRectangle(Point(0,0), Point(ws.GetX(),m_frameWidth));   // сверху
        cr->FillRectangle(Point(0,m_frameWidth), Point(m_frameWidth,ws.GetY()-m_frameWidth));   // слева
        cr->FillRectangle(Point(ws.GetX()-m_frameWidth,m_frameWidth), Point(m_frameWidth,ws.GetY()-m_frameWidth)); // справа
        cr->FillRectangle(Point(0,ws.GetY()-m_frameWidth), Point(ws.GetX(),m_frameWidth)); // снизу
    }
}

Rect &Window::GetDataRect()
{
    return m_size;
}

void Window::OnDataRectChanged(const Window *pWindow, const Rect &rect)
{
}

void Window::SetOrigin(Point origin)
{
    m_origin = origin;
}

Point Window::GetOrigin()
{
    return m_origin;
}

uint32_t  Window::PrintWindow(uint16_t level)
{
    uint32_t n = 1;

    for(uint16_t i=0; i<level; i++)
    {
        std::cout << "  ";
    }

    std::cout << m_ClassName << ": position=(" << GetPosition().GetX() << "," << GetPosition().GetY() << ")";
    std::cout << " size=(" << GetSize().GetWidth() << "," << GetSize().GetHeight() << ") visible=" << m_bShow << std::endl;

    // рекурсия в глубину
    if(m_pMyFirstChild)
    {
        n += m_pMyFirstChild->PrintWindow(level+1);
    }

    // рекурсия в ширину
    if(m_pNextChild)
    {
        n += m_pNextChild->PrintWindow(level);
    }

    return n;
}
