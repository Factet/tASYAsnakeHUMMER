// bspline.cc

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <vector>
#include "window.h"
#include "image.h"
#include "text.h"
#include "button.h"
#include "scroll.h"
#include "edit.h"
#include "GUI.h"
#include "bspline2d.h"

#define WIN_WIDTH       1000
#define WIN_HEIGHT      750
#define BACKCOLOR       RGB(0.9,0.9,0.9)
#define DLG_AREA_WIDTH  300
#define DLG_GAP         10
#define DLG_HEPHEIGHT   190
#define DLG_BUTTON_H    35
#define DLG_BUTTON_H2   50
#define DLG_POINTS_H    130
#define DLG_INFO_H      150
#define SEP_SIZE        2
#define SEP_COLOR       RGB(0.4,0.4,0.4)
#define DRW_BACKCOLOR   RGB(0.0,0.106,0.2)
#define DRW_SPACE_ADD   0.05
#define DRW_LINECOLOR   RGB(0.0,0.8,0.8)
#define DRW_LINEWIDTH   2
#define DRW_CURV_POINTS (1<<7)
#define DRW_POINT_WIDTH 8
#define DRW_CURV_WIDTH  1
#define DRW_ZOOM_FACTOR 2.0
#define BOTTOM_HEIGHT   (DLG_BUTTON_H+DLG_GAP*2)
#define FILENAME_LENGTH 255

// типы событий
enum UserEventType
{
    EVENT_NO_EVENT = 1,
    EVENT_LOAD,
    EVENT_INTERACTIVE_START,
    EVENT_INTERACTIVE_DEGREE,
    EVENT_INTERACTIVE_POINT,
    EVENT_INTERACTIVE_CLEAR,
    EVENT_INTERACTIVE_COMPLETE,
    EVENT_CREATE_BY_POLES,
    EVENT_SHOW_POLES,
    EVENT_PREV,
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class Line2D
{
public:
    Line2D(double x1, double y1, double x2, double y2, uint16_t w, RGB c) : m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_width(w), m_color(c) {}
    void GetPoint1(double *x, double *y)
    {
        *x = m_x1;
        *y = m_y1;
    }
    void GetPoint2(double *x, double *y)
    {
        *x = m_x2;
        *y = m_y2;
    }
    uint16_t GetLineWidth() { return m_width; }
    RGB GetLineColor() { return m_color; }
private:
    double m_x1, m_y1, m_x2, m_y2;
    uint16_t m_width;
    RGB m_color;
};

class Polyline2D
{
public:
    Polyline2D(uint16_t n, double points[][2], uint16_t w, RGB c) : m_width(w), m_color(c)
    {
        assert(n>=2);
        m_n = n;
        m_points = new double[2*m_n];
        for(int i=0; i<m_n; i++)
        {
            m_points[i*2+0] = points[i][0];
            m_points[i*2+1] = points[i][1];
        }
    }
    ~Polyline2D()
    {
        delete [] m_points;
        m_n = 0;
    }
    uint16_t GetNumPoints() { return m_n; }
    void GetPoint(uint16_t i, double *x, double *y)
    {
        assert(i<m_n);
        *x = m_points[2*i+0];
        *y = m_points[2*i+1];
    }
    uint16_t GetLineWidth() { return m_width; }
    RGB GetLineColor() { return m_color; }
private:
    uint16_t m_n;
    double *m_points;
    uint16_t m_width;
    RGB m_color;
};

class Point2D
{
public:
    Point2D(double x, double y, uint16_t w, RGB c) : m_x(x), m_y(y), m_width(w), m_color(c) {}
    void GetPoint(double *x, double *y)
    {
        *x = m_x;
        *y = m_y;
    }
    uint16_t GetLineWidth() { return m_width; }
    RGB GetLineColor() { return m_color; }
private:
    double m_x, m_y;
    uint16_t m_width;
    RGB m_color;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class Drawing;
class Dlg1;
class Dlg2;
class Dlg21;
class Dlg3;
class Bottom;

enum State
{
    STATE_FILE = 1,
    STATE_PARAMS,
    STATE_INTERACTIVE,
    STATE_SPLINE,
};

class MainWindow : public Window
{
public:
    MainWindow();
    ~MainWindow();

    void OnCreate();
    void OnDraw(Context *cr);
    bool OnKeyPress(uint64_t value);
    void OnSizeChanged();
    void OnNotify(Window *child, uint32_t type, const Point &position);
    void LoadPoints();
    void SetState(int state);

private:
    Text    *m_pHelp;
    Dlg1    *m_pDlg1;
    Dlg2    *m_pDlg2;
    Dlg21   *m_pDlg21;
    Dlg3    *m_pDlg3;
    Bottom  *m_pBottom;

    uint16_t m_state;
public:
    Drawing *m_pDrawing;
    char m_filename[FILENAME_LENGTH+1];
    int m_npoints;
    double (*m_points)[BSpline2D::m_nDimension];
    int m_degree;
    BSpline2D *m_pBspline;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class Dlg1 : public Window
{
public:
    Dlg1(MainWindow *pParent);
    ~Dlg1();

    void OnCreate();
    void OnNotify(Window *child, uint32_t type, const Point &position);
    void PrepareData();

private:
    MainWindow *m_pParentWindow;
    Text    *m_pText;
    Edit    *m_pEdit;
    TextButton *m_pLoad, *m_pInteractive;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class Dlg2 : public Window
{
public:
    Dlg2(MainWindow *pParent);
    ~Dlg2();

    void OnCreate();
    void OnNotify(Window *child, uint32_t type, const Point &position);
    void PrepareData();

private:
    MainWindow *m_pParentWindow;
    Text    *m_pText1, *m_pText2;
    Edit    *m_pEdit;
    TextButton *m_pButton;
    Scroll  *m_pScroll;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class Dlg21 : public Window
{
public:
    Dlg21(MainWindow *pParent);
    ~Dlg21();

    void OnCreate();
    void OnNotify(Window *child, uint32_t type, const Point &position);
    void PrepareData();
    void FinishInteractive();

private:
    MainWindow *m_pParentWindow;
    Text    *m_pInfo, *m_pDegreeLabel, *m_pTip;
    Edit    *m_pDegree;
    TextButton *m_pSetDegree, *m_pReset, *m_pFinish;
    Scroll  *m_pScroll;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class Dlg3 : public Window
{
public:
    Dlg3(MainWindow *pParent);
    ~Dlg3();

    void OnCreate();
    void OnNotify(Window *child, uint32_t type, const Point &position);
    void PrepareData();

private:
    MainWindow *m_pParentWindow;
    Text    *m_pInfo, *m_pTip;
    Scroll  *m_pScroll;
    TextButton *m_pPolygon;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class Drawing : public Window
{
public:
    Drawing(MainWindow *pParent);
    ~Drawing();

    int Load();
    void GetPoint(int i, double *x, double *y);
    void OnDraw(Context *cr);
    void OnSizeChanged();
    bool OnMouseMove(const Point &position);
    bool OnLeftMouseButtonClick(const Point &position);
    bool OnLeftMouseButtonRelease(const Point &position);
    void SetSpline(BSpline2D *pSpline);
    BSpline2D *GetSpline() { return m_pBspline; }
    void ShowSpline();
    void Clear();
    void AddLine(double p1[2], double p2[2], uint16_t width, RGB color);
    void AddPolyline(uint16_t n, double points[][2], uint16_t width, RGB color);
    void AddPoint(double p[2], uint16_t width, RGB color);
    void ShowPoles(bool);
    void SetInteractive(bool value) { m_bInteractive = value; }
    void ClearInteractive();
    void SetInteractiveDegree(int n) { m_InteractiveDegree = n; }
    void RemoveLastPole();

    void Zoom(double factor);
    void Fit();

private:
    Point ConvertToWindow(double x, double y);
    void ConvertFromWindow(Point pt, double *x, double *y);

    // алгоритм отсечения Коэна-Сазерленда
    void CohenSutherlandClipping(Context *cr, RGB color, uint16_t width, double x1, double y1, double x2, double y2);
    // генерация кода для алгоритма Коэна-Сазерленда
    uint8_t GetCohenSutherlandCode(double x, double y)
    {
        uint8_t code = 0;
        if(x < m_left)      code |= 0b0001;
        if(x > m_right)     code |= 0b0010;
        if(y < m_bottom)    code |= 0b0100;
        if(y > m_top)       code |= 0b1000;
        return code;
    }

    MainWindow *m_pParentWindow;
    double m_xmin, m_xmax, m_ymin, m_ymax;      // границы графики
    double m_left, m_right, m_top, m_bottom;    // границы окна в координатах графики
    double m_scale;                             // масштаб отображения единицы вида 2d в окно: 1мм/pix
    uint16_t m_w, m_h;                          // ширина и высота окна в пикселях
    BSpline2D *m_pBspline;
    bool m_bShowPoles;
    std::vector<Line2D*> m_Lines;
    std::vector<Polyline2D*> m_Polylines;
    std::vector<Point2D*> m_Points;
    Point m_MoveOrigin;
    bool m_bMove, m_bInteractive;
    int m_InteractiveDegree;
};


/////////////////////////////////////////////////////////////////////////////////////////////////

class Bottom : public Window
{
public:
    Bottom(MainWindow *pParent);
    ~Bottom();

    void OnCreate();
    void OnNotify(Window *child, uint32_t type, const Point &position);
    void PrepareData();

private:
    MainWindow *m_pParentWindow;
    TextButton *m_pBack;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
{
    m_ClassName = __FUNCTION__;
    m_pDrawing = nullptr;

    m_npoints = 0;
    m_state = STATE_FILE;
    strcpy(m_filename,"bspline.points");
    m_degree = 3;
    m_pBspline = nullptr;
}

MainWindow::~MainWindow()
{
    if(m_npoints>0)
    {
        delete [] m_points;
        m_points = nullptr;
    }
    if(m_pBspline)
    {
        delete m_pBspline;
        m_pBspline = nullptr;
    }
}

void MainWindow::OnCreate()
{
	Rect mysize = GetInteriorSize();
	uint16_t w = mysize.GetWidth();
	uint16_t h = mysize.GetHeight();

	SetBackColor(BACKCOLOR);

	m_pHelp = new Text(
        "Шаг 1. Выберите способ создания\n"
        "сплайна - интерактивно или\n"
        "по полюсам, загруженным из файла.\n\n"
        "Шаг 2. Задайте степень сплайна\n"
        "и постройте сплайн.\n\n"
        "Шаг 3. Исследуйте сплайн.\n\n"
        "Для возврата нажмите кнопку\n"
        "\"Назад\"."
	);
    m_pHelp->SetFont(0, 14, -1, -1);
	AddChild(m_pHelp, Point(DLG_GAP,DLG_GAP), Rect(DLG_AREA_WIDTH-DLG_GAP*2,DLG_HEPHEIGHT));

	int hh = DLG_HEPHEIGHT+2*DLG_GAP+SEP_SIZE;
	m_pDlg1 = new Dlg1(this);
	AddChild(m_pDlg1, Point(0,hh), Rect(DLG_AREA_WIDTH,h-hh-BOTTOM_HEIGHT));
	m_pDlg2 = new Dlg2(this);
	AddChild(m_pDlg2, Point(0,hh), Rect(DLG_AREA_WIDTH,h-hh-BOTTOM_HEIGHT));
	m_pDlg21 = new Dlg21(this);
	AddChild(m_pDlg21, Point(0,hh), Rect(DLG_AREA_WIDTH,h-hh-BOTTOM_HEIGHT));
	m_pDlg3 = new Dlg3(this);
	AddChild(m_pDlg3, Point(0,hh), Rect(DLG_AREA_WIDTH,h-hh-BOTTOM_HEIGHT));
	m_pBottom = new Bottom(this);
	AddChild(m_pBottom, Point(0,h-BOTTOM_HEIGHT), Rect(DLG_AREA_WIDTH,BOTTOM_HEIGHT));

    // область чертежа
	m_pDrawing = new Drawing(this);
	AddChild(m_pDrawing, Point(DLG_AREA_WIDTH+SEP_SIZE,0), Rect(w-(DLG_AREA_WIDTH+SEP_SIZE),h));
	m_pDrawing->SetBackColor(DRW_BACKCOLOR);

    CaptureKeyboard(this);
    SetState(STATE_FILE);
}

void MainWindow::OnDraw(Context *cr)
{
    Rect is = GetInteriorSize();

    cr->SetColor(GetBackColor());
    cr->FillRectangle(Point(0,0),is);

    cr->SetColor(SEP_COLOR);
    cr->FillRectangle(Point(DLG_AREA_WIDTH,0),Rect(SEP_SIZE,is.GetHeight()));
    cr->FillRectangle(Point(0,DLG_HEPHEIGHT+DLG_GAP),Rect(DLG_AREA_WIDTH,SEP_SIZE));
}

void MainWindow::OnSizeChanged()
{
	Rect mysize = GetInteriorSize();
	uint16_t w = mysize.GetWidth();
	uint16_t h = mysize.GetHeight();

	m_pDlg1->SetSize(Rect(DLG_AREA_WIDTH,h-BOTTOM_HEIGHT));
	m_pDlg2->SetSize(Rect(DLG_AREA_WIDTH,h-BOTTOM_HEIGHT));
	m_pDlg21->SetSize(Rect(DLG_AREA_WIDTH,h-BOTTOM_HEIGHT));
	m_pDlg3->SetSize(Rect(DLG_AREA_WIDTH,h-BOTTOM_HEIGHT));
	m_pDrawing->SetSize(Rect(w-(DLG_AREA_WIDTH+SEP_SIZE),h));
	m_pBottom->SetPosition(Point(0,h-BOTTOM_HEIGHT));
}

bool MainWindow::OnKeyPress(uint64_t keyval)
{
    switch(keyval)
    {
    case 'q':
        DeleteMe();
        return true;
    case 'p':
    case 'P':
        theGUI->Print();
        return true;
    case '+':
    case '=':
        m_pDrawing->Zoom(DRW_ZOOM_FACTOR);
        return true;
    case '-':
    case '_':
        m_pDrawing->Zoom(1.0/DRW_ZOOM_FACTOR);
        return true;
    case 'f':
    case 'F':
        m_pDrawing->Fit();
        return true;
    case KEY_Backspace:
        if(m_state == STATE_INTERACTIVE)
        {
            m_pDrawing->RemoveLastPole();
        }
        return true;
    case KEY_Return:
        if(m_state == STATE_INTERACTIVE)
        {
            m_pDlg21->FinishInteractive();
        }
        return true;
    default:
        ;
    }
    return true;
}

void MainWindow::OnNotify(Window *child, uint32_t type, const Point &position)
{
    if(child == m_pDlg1 && type == EVENT_LOAD)
    {
        // загрузим точки
        LoadPoints();
        if(m_npoints>2)
        {
            SetState(STATE_PARAMS);
            ReDraw();
        }
        CaptureKeyboard(this);
    }
    else if(child == m_pDlg1 && type == EVENT_INTERACTIVE_START)
    {
        // интерактивное создание
        m_pDrawing->SetSpline(m_pBspline);
        m_pDrawing->ShowPoles(true);
        SetState(STATE_INTERACTIVE);
        ReDraw();
        CaptureKeyboard(this);
    }
    else if(child == m_pDlg2 && type == EVENT_CREATE_BY_POLES)
    {
        // создаем сплайн по полюсам
        m_pBspline = new BSpline2D;
        m_pBspline->CreateByPoles(m_degree, m_npoints, m_points);
        m_pDrawing->SetSpline(m_pBspline);
        SetState(STATE_SPLINE);
        ReDraw();
        CaptureKeyboard(this);
    }
    else if(child == m_pDrawing && type == EVENT_INTERACTIVE_POINT)
    {
        // в сплайн интерактивно добавлен полюс
        m_pDlg21->PrepareData();
        CaptureKeyboard(this);
    }
    else if(child == m_pDlg21 && type == EVENT_INTERACTIVE_COMPLETE)
    {
        // интерактивное создание сплайна завершено
        m_pBspline = m_pDrawing->GetSpline();
        SetState(STATE_SPLINE);
        ReDraw();
        CaptureKeyboard(this);
    }
    else if(child == m_pBottom && type == EVENT_PREV)
    {
        // возврат назад
        if(m_state == STATE_SPLINE)
        {
            delete m_pBspline;
            m_pBspline = nullptr;
            m_pDrawing->SetSpline(nullptr);
            if(m_npoints > 0)
            {
                // точки были загружены из файла
                SetState(STATE_PARAMS);
            }
            else
            {
                // точки были введены интерактивно
                SetState(STATE_FILE);
            }
        }
        else if(m_state == STATE_PARAMS)
        {
            if(m_npoints)
            {
                delete [] m_points;
                m_npoints = 0;
                m_points = nullptr;
            }
            SetState(STATE_FILE);
        }
        else if(m_state == STATE_INTERACTIVE)
        {
            m_pDrawing->ClearInteractive();
            SetState(STATE_FILE);
        }
    }
}

void MainWindow::LoadPoints()
{
    std::ifstream input(m_filename,std::ios::in);
    if(!input.good())
    {
        std::cerr << "Файл " << m_filename << " не открыт" << std::endl;
        return;
    }

    double x, y;
    int n = 0;
    while(input >> x >> y)
    {
        n++;
    }

    if(n<2)
    {
        return;
    }

    // запомним количество полюсов и разместим массив координат полюсов
    m_npoints = n;
    m_points = new double[m_npoints][BSpline2D::m_nDimension];

    input.clear();
    input.seekg(std::ios::beg);
    for(int i=0; i<n; i++)
    {
        input >> x >> y;
        m_points[i][0] = x;
        m_points[i][1] = y;
    }
    input.close();
    std::cout << "Из файла " << m_filename << " прочитано " << n << " точек" << std::endl;
}

void MainWindow::SetState(int state)
{
    m_state = state;
    if(m_state == STATE_FILE)
    {
        m_pDlg1->Show();
        m_pDlg1->PrepareData();
        m_pDlg2->Hide();
        m_pDlg21->Hide();
        m_pDlg3->Hide();
        m_pDrawing->SetInteractive(false);
        m_pBottom->Hide();
    }
    else if(m_state == STATE_PARAMS)
    {
        m_pDlg1->Hide();
        m_pDlg2->Show();
        m_pDlg2->PrepareData();
        m_pDlg21->Hide();
        m_pDlg3->Hide();
        m_pDrawing->SetInteractive(false);
        m_pBottom->Show();
    }
    else if(m_state == STATE_INTERACTIVE)
    {
        m_pDlg1->Hide();
        m_pDlg2->Hide();
        m_pDlg21->Show();
        m_pDlg21->PrepareData();
        m_pDlg3->Hide();
        m_pDrawing->SetInteractive(true);
        m_pBottom->Show();
    }
    else if(m_state == STATE_SPLINE)
    {
        m_pDlg1->Hide();
        m_pDlg2->Hide();
        m_pDlg21->Hide();
        m_pDlg3->Show();
        m_pDlg3->PrepareData();
        m_pDrawing->SetInteractive(false);
        m_pBottom->Show();
    }
    ReDraw();
    CaptureKeyboard(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

Dlg1::Dlg1(MainWindow *pParent)
{
    m_ClassName = __FUNCTION__;
    m_pParentWindow = pParent;
    m_pLoad = nullptr;
}

Dlg1::~Dlg1()
{
}

void Dlg1::OnCreate()
{
	// кнопка интерактивного создания сплайна
	m_pInteractive = new TextButton("Создать интерактивно",EVENT_INTERACTIVE_START);
	AddChild(m_pInteractive, Point(DLG_GAP,DLG_GAP), Rect(DLG_AREA_WIDTH-2*DLG_GAP,DLG_BUTTON_H));

    m_pText = new Text("Файл:");
    m_pText->SetFont(0, 14, -1, -1);
    m_pText->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER);
	AddChild(m_pText, Point(DLG_GAP,DLG_GAP*2+DLG_BUTTON_H), Rect(50,DLG_BUTTON_H));

	m_pEdit = new Edit(m_pParentWindow->m_filename);
    m_pEdit->SetFont(0, 14, -1, -1);
    m_pEdit->SetFrameWidth(1);
	AddChild(m_pEdit, Point(DLG_GAP*2+50,DLG_GAP*2+DLG_BUTTON_H), Rect(DLG_AREA_WIDTH-3*DLG_GAP-50,DLG_BUTTON_H));

	// кнопка загрузки точек
	m_pLoad = new TextButton("Загрузить полюса",EVENT_LOAD);
	AddChild(m_pLoad, Point(DLG_GAP,DLG_GAP*3+DLG_BUTTON_H*2), Rect(DLG_AREA_WIDTH-2*DLG_GAP,DLG_BUTTON_H));
}

void Dlg1::OnNotify(Window *child, uint32_t type, const Point &position)
{
    if(child == m_pLoad && type == EVENT_LOAD)
    {
        // узнаем имя файла и оповещаем родитея
        if(m_pEdit->GetTextBytes() <= FILENAME_LENGTH)
        {
            strcpy(m_pParentWindow->m_filename, m_pEdit->GetText());
            NotifyParent(type, position);
        }
        else
        {
            std::cerr << "Ошибка: длина имени файла превышает " << FILENAME_LENGTH << " символов" << std::endl;
        }
    }
    else if(child == m_pInteractive && type == EVENT_INTERACTIVE_START)
    {
        NotifyParent(type, position);
    }
}

void Dlg1::PrepareData()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

Dlg2::Dlg2(MainWindow *pParent)
{
    m_ClassName = __FUNCTION__;
    m_pParentWindow = pParent;
}

Dlg2::~Dlg2()
{
}

void Dlg2::OnCreate()
{
	m_pScroll = new Scroll;
	AddChild(m_pScroll, Point(0,DLG_GAP), Rect(DLG_AREA_WIDTH,DLG_POINTS_H));

	// информация о точках
	m_pText1 = new Text();
    m_pText1->SetFont("Monospace", 14, -1, -1);
    m_pText1->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_TOP);
    m_pText1->SetWrap(false);
    m_pScroll->SetDataWindow(m_pText1);

    // Степень сплайна
	m_pText2 = new Text("Степень сплайна:");
    m_pText2->SetFont(0, 14, -1, -1);
    m_pText2->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER);
	AddChild(m_pText2, Point(DLG_GAP,DLG_GAP*2+DLG_POINTS_H), Rect(140,DLG_BUTTON_H));

    std::ostringstream os;
    os << m_pParentWindow->m_degree;
    m_pEdit = new Edit(os.str().c_str());
    m_pEdit->SetFont(0, 14, -1, -1);
    m_pEdit->SetFrameWidth(1);
	AddChild(m_pEdit, Point(DLG_GAP*2+140,DLG_GAP*2+DLG_POINTS_H), Rect(80,DLG_BUTTON_H));

	m_pButton = new TextButton("Создать сплайн",EVENT_CREATE_BY_POLES);
	AddChild(m_pButton, Point(DLG_GAP,DLG_GAP*3+DLG_POINTS_H+DLG_BUTTON_H), Rect(DLG_AREA_WIDTH-2*DLG_GAP,DLG_BUTTON_H));
}

void Dlg2::OnNotify(Window *child, uint32_t type, const Point &position)
{
    if(child == m_pButton && type == EVENT_CREATE_BY_POLES)
    {
        // узнаем степень сплайна
        char *str = m_pEdit->GetText(), *endp;

        long d = strtol(str, &endp, 10);
        while(*endp)
        {
            if(*endp != ' ')
            {
                std::cerr << "Ошибка преобразования в целое число: " << str << std::endl;
                return;
            }
            ++endp;
        }
        if(d<1)
        {
            std::cerr << "Неверное значение степени: " << d << std::endl;
            return;
        }
        m_pParentWindow->m_degree = d;
        NotifyParent(EVENT_CREATE_BY_POLES, position);
    }
}

void Dlg2::PrepareData()
{
    std::ostringstream os;
    os << "Загружено " << m_pParentWindow->m_npoints << " точек:";
    for(int i=0; i<m_pParentWindow->m_npoints; i++)
    {
        os << std::endl << m_pParentWindow->m_points[i][0]
            << " " << m_pParentWindow->m_points[i][1];
    }
    m_pText1->SetText(os.str().c_str());
    m_pText1->ReDraw();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

Dlg21::Dlg21(MainWindow *pParent)
{
    m_ClassName = __FUNCTION__;
    m_pParentWindow = pParent;
}

Dlg21::~Dlg21()
{
}

void Dlg21::OnCreate()
{
	m_pScroll = new Scroll;
	AddChild(m_pScroll, Point(0,DLG_GAP), Rect(DLG_AREA_WIDTH,DLG_POINTS_H));

	// информация о точках
	m_pInfo = new Text();
    m_pInfo->SetFont("Monospace", 14, -1, -1);
    m_pInfo->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_TOP);
    m_pInfo->SetWrap(false);
    m_pScroll->SetDataWindow(m_pInfo);

    // Степень сплайна
	m_pDegreeLabel = new Text("Степень:");
    m_pDegreeLabel->SetFont(0, 14, -1, -1);
    m_pDegreeLabel->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER);
	AddChild(m_pDegreeLabel, Point(DLG_GAP,DLG_GAP*2+DLG_POINTS_H), Rect(70,DLG_BUTTON_H));

    std::ostringstream os;
    os << m_pParentWindow->m_degree;
    m_pDegree = new Edit(os.str().c_str());
    m_pDegree->SetFont(0, 14, -1, -1);
    m_pDegree->SetFrameWidth(1);
	AddChild(m_pDegree, Point(DLG_GAP*2+70,DLG_GAP*2+DLG_POINTS_H), Rect(60,DLG_BUTTON_H));

	m_pSetDegree = new TextButton("Установить",EVENT_INTERACTIVE_DEGREE);
	AddChild(m_pSetDegree, Point(DLG_GAP*4+130,DLG_GAP*2+DLG_POINTS_H), Rect(DLG_AREA_WIDTH-(DLG_GAP*5+130),DLG_BUTTON_H));

	m_pReset = new TextButton("Сброс",EVENT_INTERACTIVE_CLEAR);
	AddChild(m_pReset, Point(DLG_GAP,DLG_GAP*3+DLG_POINTS_H+DLG_BUTTON_H), Rect(DLG_AREA_WIDTH-2*DLG_GAP,DLG_BUTTON_H));

	m_pFinish = new TextButton("Создать",EVENT_INTERACTIVE_COMPLETE);
	AddChild(m_pFinish, Point(DLG_GAP,DLG_GAP*4+DLG_POINTS_H+DLG_BUTTON_H*2), Rect(DLG_AREA_WIDTH-2*DLG_GAP,DLG_BUTTON_H));

    // Совет
	m_pTip = new Text();
    m_pTip->SetFont(0, 14, -1, -1);
    m_pTip->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_TOP);
	AddChild(m_pTip, Point(DLG_GAP,DLG_GAP*5+DLG_POINTS_H+DLG_BUTTON_H*3), Rect(DLG_AREA_WIDTH-2*DLG_GAP,4*DLG_BUTTON_H));
}

void Dlg21::OnNotify(Window *child, uint32_t type, const Point &position)
{
    if(child == m_pSetDegree && type == EVENT_INTERACTIVE_DEGREE)
    {
        // узнаем степень сплайна
        char *str = m_pDegree->GetText(), *endp;

        long d = strtol(str, &endp, 10);
        while(*endp)
        {
            if(*endp != ' ')
            {
                std::cerr << "Ошибка преобразования в целое число: " << str << std::endl;
                return;
            }
            ++endp;
        }
        if(d<1)
        {
            std::cerr << "Неверное значение степени: " << d << std::endl;
            return;
        }
        m_pParentWindow->m_pDrawing->SetInteractiveDegree(d);
        m_pParentWindow->m_degree = d;
    }
    else if(child == m_pReset && type == EVENT_INTERACTIVE_CLEAR)
    {
        m_pParentWindow->m_pDrawing->ClearInteractive();
        PrepareData();
        ReDraw();
    }
    if(child == m_pFinish && type == EVENT_INTERACTIVE_COMPLETE)
    {
        FinishInteractive();
    }
}

void Dlg21::PrepareData()
{
    m_pParentWindow->m_pDrawing->SetInteractiveDegree(m_pParentWindow->m_degree);

    std::ostringstream os;
    BSpline2D *pSpline = m_pParentWindow->m_pDrawing->GetSpline();

    if(pSpline)
    {
        pSpline->Print(os);
        m_pTip->SetText(
            "Для начала построения с начала\n"
            "нажмите кнопку \"Сброс\".\n"
            "Для завершения ввода точек\n"
            "и создания сплайна нажмите\n"
            "кнопку \"Создать\" или \"Enter\".\n"
            "Для удаления последнего\n"
            "указанного полюса нажмите\n"
            "клавишу \"Backspace\".\n"
        );
    }
    else
    {
        os << "Установите нужную степень сплайна,\n"
        "укажите точки полюсов и нажмите\n"
        "кнопку \"Создать\"";
        m_pTip->SetText("");
    }
    m_pInfo->SetText(os.str().c_str());
    ReDraw();
}

void Dlg21::FinishInteractive()
{
    BSpline2D *pSpline = m_pParentWindow->m_pDrawing->GetSpline();

    if(pSpline)
    {
        pSpline->RemoveLastPole();
        m_pParentWindow->m_pDrawing->ShowSpline();
        NotifyParent(EVENT_INTERACTIVE_COMPLETE, Point(0,0));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

Dlg3::Dlg3(MainWindow *pParent)
{
    m_ClassName = __FUNCTION__;
    m_pParentWindow = pParent;
}

Dlg3::~Dlg3()
{
}

void Dlg3::OnCreate()
{
	m_pScroll = new Scroll;
	AddChild(m_pScroll, Point(0,DLG_GAP), Rect(DLG_AREA_WIDTH,DLG_INFO_H));

	// информация о сплайне
	m_pInfo = new Text();
    m_pInfo->SetFont(0, 14, -1, -1);
    m_pInfo->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_TOP);
    m_pInfo->SetWrap(false);
    m_pScroll->SetDataWindow(m_pInfo);

	// кнопка отображения полюсов
	m_pPolygon = new TextButton("Показать полюса и узлы",EVENT_SHOW_POLES);
	m_pPolygon->SetToggle(true);
	m_pPolygon->SetToggleState(false);
	AddChild(m_pPolygon, Point(DLG_GAP,DLG_GAP*2+DLG_INFO_H), Rect(DLG_AREA_WIDTH-2*DLG_GAP,DLG_BUTTON_H));

    // Совет
	m_pTip = new Text(
        "Для управления отбражением\n"
        "используйте клавиши '+' (ближе),\n"
        "'-' (дальше), 'f' (показать все).\n"
        "Для сдвига вида перемещайте\n"
        "указатель при нажатой ЛКМ.\n"
	);
    m_pTip->SetFont(0, 14, -1, -1);
    m_pTip->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_TOP);
	AddChild(m_pTip, Point(DLG_GAP,DLG_GAP*3+DLG_INFO_H+DLG_BUTTON_H), Rect(DLG_AREA_WIDTH-2*DLG_GAP,3*DLG_BUTTON_H));
}

void Dlg3::OnNotify(Window *child, uint32_t type, const Point &position)
{
    if(child == m_pPolygon && type == EVENT_SHOW_POLES)
    {
        m_pParentWindow->m_pDrawing->ShowPoles(m_pPolygon->GetToggleState());
    }
}

void Dlg3::PrepareData()
{
    m_pParentWindow->m_pDrawing->ShowPoles(false);
    m_pPolygon->SetToggleState(false);

    // информация о сплайне
    std::ostringstream os;
    m_pParentWindow->m_pBspline->Print(os);
    m_pInfo->SetText(os.str().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////

Drawing::Drawing(MainWindow *pParent)
{
    m_ClassName = __FUNCTION__;
    m_pParentWindow = pParent;
    m_pBspline = nullptr;
    m_bShowPoles = false;
    m_bMove = false;
    m_bInteractive = false;
}

Drawing::~Drawing()
{
    Clear();
}

void Drawing::SetSpline(BSpline2D *pSpline)
{
    m_pBspline = pSpline;
    ShowSpline();
    Fit();
}

void Drawing::ShowSpline()
{
    Clear();

	if(m_pBspline)
	{
        int degree = m_pBspline->GetDegree();
        int npoles = m_pBspline->GetNPoles();

        // точки - вершины ломаной линии
        int npts = degree==1 ? npoles : max(1<<9,npoles);
        double points[npts+2][BSpline2D::m_nDimension];

        uint16_t k=0;
        double x, y, w, u, du=1.0/npts;

        // подготовка массива точек ломаной, отображающей сплайн
        if(degree==1)
        {
            // сплайн 1-ой степени: рисуем ломаную линию по полюсам
            for(k=0; k<npoles; k++)
            {
                m_pBspline->GetPole(k, &points[k][0], &points[k][1], &w);
                points[k][0] /= w;
                points[k][1] /= w;
            }
        }
        else
        {
            // вычисление координат ломаной линии, при помощи которой отображается сплайн
            for(u=0; u<=1; u+=du)
            {
                m_pBspline->CurvePoint(u,&points[k][0]);

                assert(k<npts+2);
                ++k;
            }
        }

        // добавим ломаную линию
        AddPolyline(k, points, DRW_LINEWIDTH, DRW_LINECOLOR);

        // отображение полюсов при помощи ломаной по полюсам
        if(m_bShowPoles && degree>1)
        {
            for(k=0; k<npoles; k++)
            {
                m_pBspline->GetPole(k, &points[k][0], &points[k][1], &w);
                points[k][0] /= w;
                points[k][1] /= w;
            }

            // добавим ломаную линию
            AddPolyline(k, points, DRW_CURV_WIDTH, DRW_LINECOLOR);

            // отображение узлов
            for(k=0; k<m_pBspline->GetNKnots(); k++)
            {
                double C[3];
                m_pBspline->CurvePoint(m_pBspline->GetKnot(k),C);
                AddPoint(C,DRW_POINT_WIDTH,DRW_LINECOLOR);
            }
        }
    }

    ReDraw();
}

void Drawing::Clear()
{
    int i, n;

    n = m_Lines.size();
    for(i=0; i<n; i++)
    {
        delete m_Lines[i];
    }
    m_Lines.clear();

    n = m_Polylines.size();
    for(i=0; i<n; i++)
    {
        delete m_Polylines[i];
    }
    m_Polylines.clear();

    n = m_Points.size();
    for(i=0; i<n; i++)
    {
        delete m_Points[i];
    }
    m_Points.clear();
}

void Drawing::AddLine(double p1[2], double p2[2], uint16_t width, RGB color)
{
    Line2D *line = new Line2D(p1[0], p1[1], p2[0],p2[1], width, color);
    m_Lines.push_back(line);
}

void Drawing::AddPolyline(uint16_t n, double points[][2], uint16_t width, RGB color)
{
    Polyline2D *polyline = new Polyline2D(n, points, width, color);
    m_Polylines.push_back(polyline);
}

void Drawing::AddPoint(double p[2], uint16_t width, RGB color)
{
    Point2D *point = new Point2D(p[0], p[1], width, color);
    m_Points.push_back(point);
}

void Drawing::ShowPoles(bool value)
{
    m_bShowPoles = value;
    ShowSpline();
}

void Drawing::ClearInteractive()
{
    if(m_pBspline)
    {
        delete m_pBspline;
        m_pBspline = nullptr;
    }
    ShowSpline();
}

void Drawing::RemoveLastPole()
{
    if(m_pBspline)
    {
        double x, y, w;
        int n = m_pBspline->GetNPoles();

        m_pBspline->GetPole(n-1, &x, &y, &w);
        m_pBspline->RemoveLastPole();
        m_pBspline->SetPole(n-2, x, y, w);

        ShowSpline();
        ReDraw();
    }
}


void Drawing::OnDraw(Context *cr)
{
	// фон
	cr->SetColor(GetBackColor());
	cr->FillRectangle(Point(0,0),Rect(m_w,m_h));

	if(m_pBspline)
	{
        int n;
        double x1, y1, x2, y2;

        // отрезки
        n = m_Lines.size();
        for(int i=0; i<n; i++)
        {
            Line2D *pLine = m_Lines[i];

            pLine->GetPoint1(&x1,&y1);
            pLine->GetPoint2(&x2,&y2);
            CohenSutherlandClipping(cr, pLine->GetLineColor(), pLine->GetLineWidth(), x1, y1, x2, y2);
        }

        // ломаные линии
        n = m_Polylines.size();
        for(int i=0; i<n; i++)
        {
            Polyline2D *pPolyline = m_Polylines[i];
            uint16_t m = pPolyline->GetNumPoints();

            pPolyline->GetPoint(0,&x2,&y2);
            for(int j=1; j<m; j++)
            {
                x1 = x2;
                y1 = y2;
                pPolyline->GetPoint(j,&x2,&y2);
                CohenSutherlandClipping(cr, pPolyline->GetLineColor(), pPolyline->GetLineWidth(), x1, y1, x2, y2);
            }
        }

        // точки
        n = m_Points.size();
        for(int i=0; i<n; i++)
        {
            Point2D *pPoint = m_Points[i];
            pPoint->GetPoint(&x1,&y1);
            if(GetCohenSutherlandCode(x1,y1)==0)
            {
                Point pt = ConvertToWindow(x1, y1);
                int w = pPoint->GetLineWidth();
                Point pt1 = pt - Rect(w/2,w/2);

                cr->SetColor(pPoint->GetLineColor());
                cr->FillRectangle(pt1,Rect(w,w));
            }
        }
    }
}

void Drawing::CohenSutherlandClipping(Context *cr, RGB color, uint16_t width, double x1, double y1, double x2, double y2)
{
    Point pt1, pt2;
    while(1)
    {
        uint8_t code1 = GetCohenSutherlandCode(x1, y1);
        uint8_t code2 = GetCohenSutherlandCode(x2, y2);

        if(code1 & code2)
        {
            // отрезок вне окна
            return;
        }
        if(!code1 && !code2)
        {
            // отрезок полностью в окне
            pt1 = ConvertToWindow(x1, y1);
            pt2 = ConvertToWindow(x2, y2);

            cr->SetColor(color);
            cr->SetLineWidth(width);
            cr->Line(pt1, pt2);

            return;
        }

        // обрабатываем всегда начало отрезка, если оно в окне, то меняем начало и конец
        if(!code1)
        {
            double tmp;
            tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
            uint8_t uc;
            uc = code1;
            code1 = code2;
            code2 = uc;
        }

        // укорачиваем отрезок (x1, y1)
        double dydx = (y2-y1)/(x2-x1);
        if(code1 & 0b0001)
        {
            x1 = m_left;
            y1 = y2 - dydx*(x2-x1);
        }
        else if(code1 & 0b0010)
        {
            x1 = m_right;
            y1 = y2 - dydx*(x2-x1);
        }

        if(code1 & 0b0100)
        {
            y1 = m_bottom;
            x1 = x2 - (y2-y1)/dydx;
        }
        else if(code1 & 0b1000)
        {
            y1 = m_top;
            x1 = x2 - (y2-y1)/dydx;
        }
    }
}

void Drawing::OnSizeChanged()
{
	Rect mysize = GetInteriorSize();
	m_w = mysize.GetWidth();
	m_h = mysize.GetHeight();

	m_right = m_left + m_w/m_scale;
	m_bottom = m_top - m_h/m_scale;
}

bool Drawing::OnMouseMove(const Point &position)
{
    if(m_bMove)
    {
        int shiftX = position.GetX(), shiftY = position.GetY();
        shiftX -= m_MoveOrigin.GetX();
        shiftY -= m_MoveOrigin.GetY();
        m_MoveOrigin = position;
        double dx = shiftX/m_scale, dy = -shiftY/m_scale;
        m_left -= dx;
        m_right -= dx;
        m_bottom -= dy;
        m_top -= dy;

        ReDraw();
        return true;
    }
    if(m_bInteractive && m_pBspline)
    {
        double x, y;
        ConvertFromWindow(position, &x, &y);
        m_pBspline->SetPole(m_pBspline->GetNPoles()-1, x, y, 1.0);

        ShowSpline();
        return true;
    }
    return false;
}

bool Drawing::OnLeftMouseButtonClick(const Point &position)
{
    if(m_bInteractive)
    {
        int np = m_pBspline ? m_pBspline->GetNPoles() : 1;
        double poles[np+1][BSpline2D::m_nDimension];
        double x, y, w, xn, yn;

        ConvertFromWindow(position, &xn, &yn);
        if(m_pBspline)
        {
            for(int i=0; i<np; i++)
            {
                m_pBspline->GetPole(i, &x, &y, &w);
                poles[i][0] = x/w;
                poles[i][1] = y/w;
            }
        }
        else
        {
            m_pBspline = new BSpline2D;
            poles[0][0] = xn;
            poles[0][1] = yn;
        }
        poles[np][0] = xn;
        poles[np][1] = yn;

        m_pBspline->CreateByPoles(m_InteractiveDegree,np+1,poles);
        ShowSpline();
        NotifyParent(EVENT_INTERACTIVE_POINT, position);
    }
    else
    {
        m_bMove = true;
        m_MoveOrigin = position;
    }
    return true;
}

bool Drawing::OnLeftMouseButtonRelease(const Point &position)
{
    if(m_bMove)
    {
        m_bMove = false;
        CaptureMouse(m_pParentWindow);
        return true;
    }
    return false;
}

Point Drawing::ConvertToWindow(double x, double y)
{
    int ix, iy;
    ix = (x-m_left)*m_scale;
    iy = m_h-(y-m_bottom)*m_scale;
    return Point(ix,iy);
}

void Drawing::ConvertFromWindow(Point pt, double *x, double *y)
{
    *x = pt.GetX()/m_scale + m_left;
    *y = (m_h-pt.GetY())/m_scale + m_bottom;
}

// ближе/дальше
void Drawing::Zoom(double factor)
{
    double f1 = 0.5*(factor-1.0)/factor;
    double dx = (m_right-m_left)*f1;
    double dy = (m_top-m_bottom)*f1;
    m_scale *= factor;
    m_left += dx;
    m_right -= dx;
    m_bottom += dy;
    m_top -= dy;
    ReDraw();
}

// показать все
void Drawing::Fit()
{
    int n;
    double x, y, tmp;

    if(m_pBspline)
    {
        m_pBspline->GetPole(0, &x, &y, &tmp);
        m_xmin = x;
        m_xmax = x;
        m_ymin = y;
        m_ymax = y;

        n = m_Points.size();
        for(int i=0; i<n; i++)
        {
            Point2D *pPoint = m_Points[i];
            pPoint->GetPoint(&x,&y);
            if(x < m_xmin) m_xmin = x;
            if(x > m_xmax) m_xmax = x;
            if(y < m_ymin) m_ymin = y;
            if(y > m_ymax) m_ymax = y;
        }
        n = m_Lines.size();
        for(int i=0; i<n; i++)
        {
            Line2D *pLine = m_Lines[i];
            pLine->GetPoint1(&x,&y);
            if(x < m_xmin) m_xmin = x;
            if(x > m_xmax) m_xmax = x;
            if(y < m_ymin) m_ymin = y;
            if(y > m_ymax) m_ymax = y;
            pLine->GetPoint2(&x,&y);
            if(x < m_xmin) m_xmin = x;
            if(x > m_xmax) m_xmax = x;
            if(y < m_ymin) m_ymin = y;
            if(y > m_ymax) m_ymax = y;
        }
        n = m_Polylines.size();
        for(int i=0; i<n; i++)
        {
            Polyline2D *pPolyline = m_Polylines[i];
            int k = pPolyline->GetNumPoints();
            for(int j=0; j<k; j++)
            {
                pPolyline->GetPoint(j,&x,&y);
                if(x < m_xmin) m_xmin = x;
                if(x > m_xmax) m_xmax = x;
                if(y < m_ymin) m_ymin = y;
                if(y > m_ymax) m_ymax = y;
            }
        }

        double dx = m_xmax-m_xmin;
        double dy = m_ymax-m_ymin;

        m_xmin -= dx*DRW_SPACE_ADD;
        m_xmax += dx*DRW_SPACE_ADD;
        m_ymin -= dy*DRW_SPACE_ADD;
        m_ymax += dy*DRW_SPACE_ADD;
    }
    else
    {
        m_xmin = -1.0;
        m_xmax = 1.0;
        m_ymin = -1.0;
        m_ymax = 1.0;
    }

    // выбор масштаба
	Rect mysize = GetInteriorSize();
	uint16_t w = mysize.GetWidth();
	uint16_t h = mysize.GetHeight();

    double dx = m_xmax-m_xmin;
    double sx = w/dx;
    double dy = m_ymax-m_ymin;
    double sy = h/dy;

    double y0 = m_ymin;
    double delta;

    if(sx<sy)
    {
        m_scale = sx;
        double wh = h/m_scale;
        double delta = 0.5*(wh-dy);
        m_bottom = m_ymin - delta;
        m_top = m_ymax + delta;
        m_left = m_xmin;
        m_right = m_xmax;
    }
    else
    {
        m_scale = sy;
        double ww = w/m_scale;
        double delta = 0.5*(ww-dx);
        m_left = m_xmin - delta;
        m_right = m_xmax + delta;
        m_bottom = m_ymin;
        m_top = m_ymax;
    }

    ReDraw();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

Bottom::Bottom(MainWindow *pParent)
{
    m_ClassName = __FUNCTION__;
    m_pParentWindow = pParent;
}

Bottom::~Bottom()
{
}

void Bottom::OnCreate()
{
	// кнопка Назад
	m_pBack = new TextButton("Назад",EVENT_PREV);
	AddChild(m_pBack, Point(DLG_GAP,DLG_GAP), Rect(DLG_AREA_WIDTH-2*DLG_GAP,DLG_BUTTON_H));
}

void Bottom::OnNotify(Window *child, uint32_t type, const Point &position)
{
    NotifyParent(type, position);
}

void Bottom::PrepareData()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    MainWindow *pWindow = new MainWindow;
    int res = Run(argc, argv, pWindow, WIN_WIDTH, WIN_HEIGHT);
    delete pWindow;
    return res;
}

