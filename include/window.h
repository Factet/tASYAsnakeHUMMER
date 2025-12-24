#include <inttypes.h>
#include "mytypes.h"
#include "context.h"

enum WindowEventTypes
{
    EVENT_UNKNOWN = 0,
    EVENT_LEFTMOUSEBUTTONCLICK,
    EVENT_RIGHTMOUSEBUTTONCLICK,
    EVENT_LEFTMOUSEBUTTONDOUBLECLICK,
    EVENT_RIGHTMOUSEBUTTONDOUBLECLICK,
    EVENT_LEFTMOUSEBUTTONRELEASE,
    EVENT_RIGHTMOUSEBUTTONRELEASE,
    EVENT_TIMEOUT,
    EVENT_MOUSEMOVE,
    EVENT_KEYPRESS,
    EVENT_WINDOWRESIZE,
    EVENT_KEYBOARDCAPTURE,
    EVENT_KEYBOARDRELEASE,
    EVENT_SCROLL,
//    ...
    EVENT_LASTNUMBER
};

class Window
{
public:
    Window();
    virtual ~Window();

    const char  *GetClassName() { return m_ClassName; }

    bool        WindowProc(uint32_t type, const Point &position, uint64_t value); // процедура окна; вызывается из ОС для обработки событий

    void        Draw(Context *cr);                                  // отрисовка окна
    virtual void OnDraw(Context *cr);                               // виртуальный метод отрисовки; должен быть переопеределен в наследующих классах
    virtual void ReDraw();                                          // запрос на перерисовывание окна

    void        Create(Window *parent);                             // создание окна; вызывается из ОС для обработки создания окна
    virtual void OnCreate();                                        // виртуальный метод создания; может быть переопределен в наследующих классах

    void        AddChild(Window *child, const Point &position, const Rect &size); // создание дочернего окна
    void        DeleteChild(Window *pChild);                        // удаление окна из потомков и вызов Destroy()
    void        DeleteAllChildren();                                // удаление и уничтожение всех потомков
    void        DeleteMe() { m_bToBeDeleted = true; }               // обращение окна к родителю: запрос об удалении
private:
    void        Destroy(Window *pChild);                            // уничтожение окна и его потомков
    void        DrawFrame(Context *cr);                             // рисование рамки
public:

    void        NotifyParent(uint32_t type, const Point &position); // уведомление родителя о событии дочернего окна
    void        Notify(Window *child, uint32_t type, const Point &position); // уведомить дочернее окно о некотором событии
    virtual void OnNotify(Window *child, uint32_t type, const Point &position); // виртуальный метод обработки уведомлений

    Window *GetParent() { return m_pParent; }                       // возвращает указатель на родительское окно
    Point       &GetPosition();                                     // возвращает положение окна
    void        SetPosition(const Point &position);                 // устанавливает положение окна в родительском окне
    Rect        &GetSize();                                         // возвращает размер окна
    virtual Rect &GetInteriorSize();                                // возвращает размер внутренней части окна (без рамки, заголовка, статуса и т.п.)
    void        SetSize(const Rect &newsize);                       // вызывается из ОС; устанавливает/изменяет размер окна
    virtual void OnSizeChanged();                                   // виртуальный метод обработки изменения размера окна
    virtual void CreateTimeout(Window *pWindow, uint32_t timeout);  // создание оповещения о таймауте
    virtual void CaptureKeyboard(Window *pWindow);                  // ретрансляция родителю о захвате клавиатуры
    virtual void CaptureMouse(Window *pWindow);                     // ретрансляция родителю о захвате мыши (для буксировки при скролинге, перемещении окна и т.п.)
    virtual bool HasKeyboard(Window *pWindow);                      // проверка, является ли окно владельцем клавиатуры

    // прокрутка
    virtual Rect &GetDataRect();                                    // возвращает размер данных в окне - м.б. больше окна
    virtual void OnDataRectChanged(const Window *pWindow, const Rect &rect); // оповещение об изменении размера окна с данными
    void        SetOrigin(Point origin);                            // установка положения окна в документе - для прокрутки
    Point       GetOrigin();                                        // возврат положения окна в документе - для прокрутки

    // рамка
    RGB  GetFrameColor();
    void SetFrameColor(const RGB frameColor);
    uint16_t GetFrameWidth();
    void SetFrameWidth(const uint16_t frameWidth);

    // фон
    RGB  GetBackColor();
    void SetBackColor(const RGB backColor);

    // управление отображением окна
    void Show() { m_bShow = true; }
    void Hide() { m_bShow = false; }

    // печать структуры окон
    uint32_t PrintWindow(uint16_t level=0);

public:

    // пустые виртуальные обработчики событий
    virtual bool OnLeftMouseButtonClick(const Point &position) { return false; }
    virtual bool OnRightMouseButtonClick(const Point &position) { return false; }
    virtual bool OnLeftMouseButtonDoubleClick(const Point &position) { return false; }
    virtual bool OnRightMouseButtonDoubleClick(const Point &position) { return false; }
    virtual bool OnLeftMouseButtonRelease(const Point &position) { return false; }
    virtual bool OnRightMouseButtonRelease(const Point &position) { return false; }
    virtual bool OnMouseMove(const Point &position) { return false; }
    virtual bool OnKeyPress(uint64_t) { return false; }
    virtual bool OnTimeout() { return false; }
    virtual bool OnKeyboardCapture(bool bCapture) { return false; }
    virtual bool OnScroll(uint64_t value) { return false; }
//    ...

private:
    Rect    m_size;                                                 // размер окна
    Point   m_position;                                             // положение окна в родительском окне
    Point   m_origin;                                               // положение ЛВУ окна в документе - для прокрутки
    Window  *m_pParent;                                             // родительское окно
    Window  *m_pMyFirstChild;                                       // начало цепочки дочерних окон
    Window  *m_pNextChild;                                          // следующее дочернее окно в цепочке дочерних окон родителя
    bool    m_bCreated;                                             // окно создано
    RGB     m_frameColor;                                           // цвет рамки
    uint16_t m_frameWidth;                                          // толщина рамки
    bool    m_bShow;                                                // отображение окна
protected:
    RGB     m_backColor;                                            // цвет фона
    Rect    m_InteriorSize;                                         // размер внутренней части окна (без рамки, меню, статуса, заголовка и т.п.)
public:
    bool    m_bToBeDeleted;                                         // запрос удаления окна
    const char    *m_ClassName;
};

