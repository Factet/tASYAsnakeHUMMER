#include <cmath>
#include <iostream>

#include "window.h"
#include "text.h"
#include "image.h"
#include "button.h"
#include "edit.h"
#include "GUI.h"

// типы событий
enum UserEventType
{
    EVENT_CLOSE = 1,
    EVENT_SOLVE,
};

class MainWindow : public Window
{
public:
    MainWindow() {}
    ~MainWindow() {}

    void OnCreate();
    void OnDraw(Context *cr);
    bool OnKeyPress(uint64_t value);
    bool OnLeftMouseButtonClick(const Point &position);
    void OnNotify(Window *child, uint32_t type, const Point &position);

    void Solve();

private:
    Text *m_pHeader, *m_pA, *m_pB, *m_pC, *m_pSolution;
    Edit *m_pValueA, *m_pValueB, *m_pValueC;
};

void MainWindow::OnDraw(Context *cr)
{
	Point mysize = GetInteriorSize();

    // зальем прямоугольник окна серым цветом
    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(0,0), mysize);
}

void MainWindow::OnCreate()
{
    std::cout << "MainWindow::OnCreate()" << std::endl;
	Point mysize = GetInteriorSize();

	m_pHeader = new Text("Решение квадратных уравнений     A·x² + B·x + C = 0");
    AddChild(m_pHeader,Point(30,30),Rect(540,30));
    m_pHeader->SetFont(0, 16, TEXT_STYLE_BOLD, -1);

    // коэффициент A
    m_pA = new Text("Коэффициент А:");
    AddChild(m_pA,Point(130,80),Rect(150,30));
    m_pA->SetFont(0, 14, -1, -1);
    m_pA->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER);
    m_pValueA = new Edit("1.0");
    AddChild(m_pValueA,Point(290,80),Rect(150,30));
    m_pValueA->SetFont(0, 14, -1, -1);
    m_pValueA->SetBackColor(RGB_WHITE);

    // коэффициент B
    m_pB = new Text("Коэффициент B:");
    AddChild(m_pB,Point(130,130),Rect(150,30));
    m_pB->SetFont(0, 14, -1, -1);
    m_pB->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER);
    m_pValueB = new Edit("1.0");
    AddChild(m_pValueB,Point(290,130),Rect(150,30));
    m_pValueB->SetFont(0, 14, -1, -1);
    m_pValueB->SetBackColor(RGB_WHITE);

    // коэффициент C
    m_pC = new Text("Коэффициент C:");
    AddChild(m_pC,Point(130,180),Rect(150,30));
    m_pC->SetFont(0, 14, -1, -1);
    m_pC->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER);
    m_pValueC = new Edit("-1.0");
    AddChild(m_pValueC,Point(290,180),Rect(150,30));
    m_pValueC->SetFont(0, 14, -1, -1);
    m_pValueC->SetBackColor(RGB_WHITE);

    char text[100];
    lconv *plc = localeconv();
    sprintf(text,"Здесь будет решение\nЛокаль числовых данных: %s, десятичная точка '%s'",
        setlocale(LC_NUMERIC,nullptr), plc->decimal_point);
	m_pSolution = new Text(text);
    AddChild(m_pSolution,Point(30,230),Rect(540,60));
    m_pSolution->SetFont(0, 14, -1, -1);
    m_pSolution->SetFrameWidth(1);
    m_pSolution->SetAlignment(TEXT_ALIGNH_CENTER|TEXT_ALIGNV_CENTER);
    m_pSolution->SetWrap(true);

    AddChild(new TextButton("Решить",EVENT_SOLVE), Point(150, 310), Rect(100,30));
    AddChild(new TextButton("Закрыть",EVENT_CLOSE), Point(350, 310), Rect(100,30));

    CaptureKeyboard(this);
}

bool MainWindow::OnLeftMouseButtonClick(const Point &position)
{
    CaptureKeyboard(this);
    return true;
}

void MainWindow::OnNotify(Window *child, uint32_t type, const Point &position)
{
    if(type == EVENT_CLOSE)
    {
        DeleteMe();
    }
    else if(type == EVENT_SOLVE)
    {
        Solve();
    }
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
    default:
        ;
    }
    return true;
}

void MainWindow::Solve()
{
    char *endp;
    char *str, text[100];

    str = m_pValueA->GetText();
    double a = strtod(str, &endp);
    while(*endp)
    {
        if(*endp != ' ')
        {
            m_pSolution->SetText("Ошибка преобразования коэффициента A в число с плавающей точкой");
            ReDraw();
            return;
        }
        ++endp;
    }

    if(a==0)
    {
        m_pSolution->SetText("Коэффициент A не должен быть равен 0");
        ReDraw();
        return;
    }

    str = m_pValueB->GetText();
    double b = strtod(str, &endp);
    while(*endp)
    {
        if(*endp != ' ')
        {
            m_pSolution->SetText("Ошибка преобразования коэффициента B в число с плавающей точкой");
            ReDraw();
            return;
        }
        ++endp;
    }

    str = m_pValueC->GetText();
    double c = strtod(str, &endp);
    while(*endp)
    {
        if(*endp != ' ')
        {
            m_pSolution->SetText("Ошибка преобразования коэффициента C в число с плавающей точкой");
            ReDraw();
            return;
        }
        ++endp;
    }

    double d = b*b - 4*a*c;
    if(d<0)
    {
        m_pSolution->SetText("Действительных корней нет");
    }
    else if(d==0)
    {
        double x = -b/(2*a);
        sprintf(text, "Один действительный корень x=%lf",x);
        m_pSolution->SetText(text);
    }
    else
    {
        double sqd = sqrt(d);
        double x1 = (-b+sqd)/(2*a);
        double x2 = (-b-sqd)/(2*a);
        sprintf(text, "Два действительных корня: x₁=%lf, x₂=%lf",x1, x2);
        m_pSolution->SetText(text);
    }

    ReDraw();
}

// функция main
int main(int argc, char **argv)
{
    MainWindow *pWindow = new MainWindow;

    int res = Run(argc, argv, pWindow, 600, 360);

    delete pWindow;

    return res;
}
