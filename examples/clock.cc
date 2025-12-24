#include <cmath>
#include <ctime>
#include <iostream>
#include <cstring>

#include "window.h"
#include "digit7.h"
#include "GUI.h"

#define GAP 20

class MainWindow : public Window
{
public:
    MainWindow() {}
    ~MainWindow() {}

    void OnCreate();
    void OnDraw(Context *cr);
    void OnSizeChanged();
    bool OnKeyPress(uint64_t value);
    bool OnTimeout();
    void AdjustControls();
    void SetDigits();

private:
    RGB         m_foreground;
    Digit7      *m_pHH, *m_pH, *m_pMM, *m_pM, *m_pSS, *m_pS;
};

void MainWindow::OnDraw(Context *cr)
{
	std::cout << "MainWindow::OnDraw()" << std::endl;

    // зальем прямоугольник окна цветом фона
    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(0,0), GetInteriorSize());
}

void MainWindow::OnCreate()
{
    // начальные цвета
    m_foreground = RGB(0.0,0.0,0.6);

    Point pt(20,20);
    Rect r(100,200);

    m_pHH = new Digit7(0);
    m_pHH->SetColor(m_foreground);
    AddChild(m_pHH, pt, r);

    m_pH = new Digit7(0);
    m_pH->SetColor(m_foreground);
    AddChild(m_pH, pt, r);

    m_pMM = new Digit7(0);
    m_pMM->SetColor(m_foreground);
    AddChild(m_pMM, pt, r);

    m_pM = new Digit7(0);
    m_pM->SetColor(m_foreground);
    AddChild(m_pM, pt, r);

    m_pSS = new Digit7(0);
    m_pSS->SetColor(m_foreground);
    AddChild(m_pSS, pt, r);

    m_pS = new Digit7(0);
    m_pS->SetColor(m_foreground);
    AddChild(m_pS, pt, r);

	AdjustControls();
	SetDigits();
    CreateTimeout(this,1000);

    // фокус ввода
    CaptureKeyboard(this);
}

void MainWindow::OnSizeChanged()
{
	std::cout << "MainWindow::OnSizeChanged()" << std::endl;

    AdjustControls();
}

bool MainWindow::OnKeyPress(uint64_t keyval)
{
    if(keyval == 'q')
    {
        DeleteMe();
    }
    else if(keyval == 'p' || keyval == 'P')
    {
        theGUI->Print();
    }
    return true;
}

bool MainWindow::OnTimeout()
{
	std::cout << "MainWindow::OnTimeout()" << std::endl;

	SetDigits();
	ReDraw();
    return true;
}

void MainWindow::SetDigits()
{
	time_t ct = time(NULL);
    struct tm *t = localtime(&ct);
    m_pHH->SetDigit(t->tm_hour/10);
    m_pH->SetDigit(t->tm_hour%10);
    m_pMM->SetDigit(t->tm_min/10);
    m_pM->SetDigit(t->tm_min%10);
    m_pSS->SetDigit(t->tm_sec/10);
    m_pS->SetDigit(t->tm_sec%10);
}

void MainWindow::AdjustControls()
{
    Point mysize = GetInteriorSize();
    uint16_t sx = (mysize.GetX() - 13*GAP)/6, sy = mysize.GetY() - 2*GAP;
    Rect digitsize(sx, sy);

    m_pHH->SetSize(digitsize);
    m_pH->SetSize(digitsize);
    m_pMM->SetSize(digitsize);
    m_pM->SetSize(digitsize);
    m_pSS->SetSize(digitsize);
    m_pS->SetSize(digitsize);

    m_pHH->SetPosition(Point(GAP,GAP));
    m_pH->SetPosition(Point(2*GAP+sx,GAP));
    m_pMM->SetPosition(Point(6*GAP+2*sx,GAP));
    m_pM->SetPosition(Point(7*GAP+3*sx,GAP));
    m_pSS->SetPosition(Point(10*GAP+4*sx,GAP));
    m_pS->SetPosition(Point(11*GAP+5*sx,GAP));
}

// функция main
int main(int argc, char **argv)
{
    MainWindow *pWindow = new MainWindow;

    int res = Run(argc, argv, pWindow, 800, 250);

    delete pWindow;

    return res;
}
