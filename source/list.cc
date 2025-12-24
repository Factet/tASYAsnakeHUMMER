// class List

#include <cstdlib>
#include "window.h"
#include "list.h"

List::List()
{
    m_ClassName = __FUNCTION__;
    m_nElements = 0;
    m_pElements = nullptr;
    m_pValues = nullptr;
    m_ElementHeight = 18;
    m_maxWidth = 0;
    m_nSelection = -1;
    m_selbackColor = RGB(0.8,1.0,0.8);
}

List::~List()
{
    if(m_nElements>0)
    {
        free(m_pElements);
        free(m_pValues);
    }
}

uint16_t List::GetElementHeight() const
{
    return m_ElementHeight;
}

void List::SetElementHeight(const uint16_t height)
{
    m_ElementHeight = height;
}

int16_t List::GetSelection() const
{
    return m_nSelection;
}

void List::SetSelection(const int16_t n)
{
    if(n <= m_nElements)
    {
        if(m_nSelection>=0)
        {
            m_pElements[m_nSelection]->SetBackColor(m_backColor);
        }

        m_nSelection = n;

        if(m_nSelection>=0)
        {
            m_pElements[m_nSelection]->SetBackColor(m_selbackColor);
        }
    }
}

RGB  List::GetSelBackColor() const
{
    return m_selbackColor;
}

void List::SetSelBackColor(const RGB Color)
{
    m_selbackColor = Color;
}

uint16_t List::GetNumberOfElements() const
{
    return m_nElements;
}

Window *List::GetElement(const uint16_t n) const
{
    return n<m_nElements ? m_pElements[n] : nullptr;
}

void *List::GetValue(const uint16_t n) const
{
    return n<m_nElements ? m_pValues[n] : nullptr;
}

void List::Clear()
{
    while(m_nElements>0)
    {
        Delete(0);
    }

    SetOrigin(Point(0,0));
}

void List::Insert(const uint16_t position, Window *pElement, void *value)
{
    // корректируем позицию, если нужно
    uint16_t pos = position < m_nElements ? position : m_nElements;

    // отменим выбор текущего элемента
    SetSelection(-1);

    // расширяем память
    if(m_nElements == 0)
    {
        m_pElements = (Window **) malloc(sizeof(Window*));
        m_pValues = (void **) malloc(sizeof(void*));
    }
    else
    {
        m_pElements = (Window **) realloc(m_pElements,sizeof(Window*)*(m_nElements+1));
        m_pValues = (void **) realloc(m_pValues,sizeof(void*)*(m_nElements+1));
    }

    // сдвигаем указатели существующих элементов
    for(uint16_t i=m_nElements; i>pos; i--)
    {
        m_pElements[i] = m_pElements[i-1];
        m_pValues[i] = m_pValues[i-1];
    }

    // запоминаем указатель и новое количество элементов
    m_pElements[pos] = pElement;
    m_pValues[pos] = value;
    ++m_nElements;

    // добавляем окно-потомок
    Point p = Point(0, pos*m_ElementHeight);
    Rect s = Rect(GetInteriorSize().GetWidth(), m_ElementHeight);
    AddChild(pElement,p,s);

    // устанавливаем для элемента общий цвет фона
    pElement->SetBackColor(GetBackColor());

    // поместим эементы на их места
    RepositionElements();

    GetParent()->OnDataRectChanged(this,m_BackgroundSize);
}

void List::Delete(const uint16_t n)
{
    if(n >= m_nElements)
    {
        return;
    }

    // удаление потомка
    DeleteChild(m_pElements[n]);

    // если элемент был выбран, то отменим его выбор
    if(m_nSelection == n)
    {
        SetSelection(-1);
    }

    // переместим указатели
    for(uint16_t i=n; i<m_nElements-1; i++)
    {
        m_pElements[i] = m_pElements[i+1];
        m_pValues[i] = m_pValues[i+1];
    }

    // если переместили выбранный элемент, сменим выбор
    if(m_nSelection > n)
    {
        --m_nSelection;
    }

    // уменьшим количество элементов и сократим память
    --m_nElements;
    if(m_nElements == 0)
    {
        free(m_pElements);
        free(m_pValues);
        m_pElements = nullptr;
        m_pValues = nullptr;
    }
    else
    {
        m_pElements = (Window **) realloc(m_pElements,sizeof(Window*)*(m_nElements));
        m_pValues = (void **) realloc(m_pValues,sizeof(void*)*(m_nElements));
    }

    // поместим эементы на их места
    RepositionElements();

    GetParent()->OnDataRectChanged(this,m_BackgroundSize);
}

void List::RepositionElements()
{
    uint16_t h, w=0;

    for(uint16_t i=0; i<m_nElements; i++)
    {
        m_pElements[i]->SetPosition(Point(0,i*m_ElementHeight));
        Rect r = m_pElements[i]->GetDataRect();
        w = max(w,r.GetWidth());
    }

    Rect is = GetInteriorSize();
    h = max(m_nElements*m_ElementHeight, is.GetHeight());
    w = max(w, is.GetWidth());

    bool bNotify = (w != m_BackgroundSize.GetWidth()) || (h != m_BackgroundSize.GetHeight());
    m_BackgroundSize.SetWidth(w);
    m_BackgroundSize.SetHeight(h);

    if(bNotify)
    {
        GetParent()->OnDataRectChanged(this,m_BackgroundSize);
    }
}


bool List::OnLeftMouseButtonClick(const Point &position)
{
    // номер элемента
    uint16_t n = position.GetY()/m_ElementHeight;

    if(n<m_nElements)
    {
        // выберем этот элемент
        SetSelection(n);

        ReDraw();

        // оповестим родителя
        NotifyParent(EVENT_LIST_LMB_CLICK,position);
    }
    return true;
}


bool List::OnLeftMouseButtonDoubleClick(const Point &position)
{
    // номер элемента
    uint16_t n = position.GetY()/m_ElementHeight;

    if(n<m_nElements)
    {
        // оповестим родителя
        NotifyParent(EVENT_LIST_LMB_DOUBLECLICK,position);
    }
    return true;
}


void List::OnDraw(Context *cr)
{
    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(0,0),m_BackgroundSize);
}

void List::OnSizeChanged()
{
    RepositionElements();
}

Rect &List::GetDataRect()
{
    m_DataRect = m_BackgroundSize;
    return m_DataRect;
}

void List::OnDataRectChanged(const Window *pWindow, const Rect &d)
{
    RepositionElements();
}

