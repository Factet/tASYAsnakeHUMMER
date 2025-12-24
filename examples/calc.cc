#include <cmath>
#include <iostream>
#include <string.h>

#include "window.h"
#include "text.h"
#include "image.h"
#include "button.h"
#include "GUI.h"

#define BUTTON_W    40
#define BUTTON_H    60
#define BUTTON_SPACE 10
#define BUTTON_TOP  100
#define BUTTON_LEFT 50

// типы событий
enum UserEventType
{
    EVENT_0 = 1,
    EVENT_1,
    EVENT_2,
    EVENT_3,
    EVENT_4,
    EVENT_5,
    EVENT_6,
    EVENT_7,
    EVENT_8,
    EVENT_9,
    EVENT_ADD,
    EVENT_SUBTRACT,
    EVENT_MULTIPLY,
    EVENT_DIVIDE,
    EVENT_EQ,
    EVENT_RESET,
};

// состояния калькулятора
enum Calc_State
{
    STATE_EMPTY = 1,    // ничего не введено
    STATE_VAL1,         // выполняется ввод первого числа
    STATE_ACTION,       // введено действие + - * /
    STATE_VAL2,         // выполняется ввод второго числа
    STATE_EQ,           // введен символ =
};

// действия
enum Action
{
    ACTION_ADD = 1,
    ACTION_SUBTRACT,
    ACTION_MULTIPLY,
    ACTION_DIVIDE,
};

// родительское окно
class MainWindow : public Window
{
public:
    MainWindow() {     m_ClassName = __FUNCTION__; m_string[0]=0; }
    ~MainWindow() {}

    void OnCreate();
    void OnDraw(Context *cr);
    void OnSizeChanged();
    void OnNotify(Window *child, uint32_t type, const Point &position);
    bool OnLeftMouseButtonClick(const Point &Position);
    bool OnKeyPress(uint64_t value);

    void ProcessDigit(uint16_t digit);
    void RememberAction(uint16_t action);
    void Compute();
    void PrintString();

private:
    int32_t     m_value1, m_value2, m_res;
    uint16_t    m_state, m_action;
    char        m_string[80];
    Text        *m_pText;
};

void MainWindow::OnDraw(Context *cr)
{
    std::cout << "MainWindow::OnDraw()" << std::endl;
	Point wsize = GetInteriorSize();

    // зальем прямоугольник окна серым цветом
    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(0,0), wsize);
}

void MainWindow::OnCreate()
{
    std::cout << "MainWindow::OnCreate()" << std::endl;

    // текст
    m_pText = new Text;
    m_pText->SetFrameWidth(1);
    m_pText->SetFont(NULL,20,-1,-1);
    m_pText->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER);
    m_pText->SetBackColor(RGB(1.0,1.0,1.0));
    m_pText->SetWrap(true);
    AddChild(m_pText,Point(BUTTON_LEFT/2, BUTTON_TOP/3),Rect(320, 50));
    m_pText->SetBackColor(RGB(0.8,1.0,0.8));

    // добавляем кнопки - первый ряд
    AddChild(new TextButton("0",EVENT_0),           Point(BUTTON_LEFT+0*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("1",EVENT_1),           Point(BUTTON_LEFT+1*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("2",EVENT_2),           Point(BUTTON_LEFT+2*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("3",EVENT_3),           Point(BUTTON_LEFT+3*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP), Rect(BUTTON_H,BUTTON_W));

    // добавляем кнопки - второй ряд
    AddChild(new TextButton("4",EVENT_4),           Point(BUTTON_LEFT+0*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+1*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("5",EVENT_5),           Point(BUTTON_LEFT+1*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+1*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("6",EVENT_6),           Point(BUTTON_LEFT+2*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+1*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("7",EVENT_7),           Point(BUTTON_LEFT+3*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+1*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));

    // добавляем кнопки - третий ряд
    AddChild(new TextButton("8",EVENT_8),           Point(BUTTON_LEFT+0*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+2*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("9",EVENT_9),           Point(BUTTON_LEFT+1*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+2*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("+",EVENT_ADD),         Point(BUTTON_LEFT+2*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+2*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("-",EVENT_SUBTRACT),    Point(BUTTON_LEFT+3*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+2*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));

    // добавляем кнопки - четвертый ряд
    AddChild(new TextButton("*",EVENT_MULTIPLY),    Point(BUTTON_LEFT+0*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+3*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("/",EVENT_DIVIDE),      Point(BUTTON_LEFT+1*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+3*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("=",EVENT_EQ),          Point(BUTTON_LEFT+2*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+3*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));
    AddChild(new TextButton("Clear",EVENT_RESET),   Point(BUTTON_LEFT+3*(BUTTON_H+BUTTON_SPACE),BUTTON_TOP+3*(BUTTON_W+BUTTON_SPACE)), Rect(BUTTON_H,BUTTON_W));

    // начальное состояние
    m_state = STATE_EMPTY;

    // фокус ввода
    CaptureKeyboard(this);
}

void MainWindow::OnSizeChanged()
{
    std::cout << "MainWindow::OnSizeChanged()" << std::endl;
}

void MainWindow::OnNotify(Window *child, uint32_t type, const Point &position)
{
    std::cout << "MainWindow::OnNotify()" << std::endl;

    switch(type)
    {
    case EVENT_0:
    case EVENT_1:
    case EVENT_2:
    case EVENT_3:
    case EVENT_4:
    case EVENT_5:
    case EVENT_6:
    case EVENT_7:
    case EVENT_8:
    case EVENT_9:
        ProcessDigit(type-EVENT_0);
        break;
    case EVENT_ADD:
        RememberAction(ACTION_ADD);
        break;
    case EVENT_SUBTRACT:
        RememberAction(ACTION_SUBTRACT);
        break;
    case EVENT_MULTIPLY:
        RememberAction(ACTION_MULTIPLY);
        break;
    case EVENT_DIVIDE:
        RememberAction(ACTION_DIVIDE);
        break;
    case EVENT_EQ:
        Compute();
        break;
    case EVENT_RESET:
        m_state = STATE_EMPTY;
        PrintString();
        break;
    default: ;
    }
}

bool MainWindow::OnLeftMouseButtonClick(const Point &Position)
{
//    CaptureKeyboard(this);
    return true;
}

bool MainWindow::OnKeyPress(uint64_t keyval)
{
    std::cout << "MainWindow::OnKeyPress(" << keyval << ")" << std::endl;
    switch(keyval)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        ProcessDigit(keyval-'0');
        break;
    case '+':
        RememberAction(ACTION_ADD);
        break;
    case '-':
        RememberAction(ACTION_SUBTRACT);
        break;
    case '*':
        RememberAction(ACTION_MULTIPLY);
        break;
    case '/':
        RememberAction(ACTION_DIVIDE);
        break;
    case '=':
    case KEY_Return:
        Compute();
        break;
    case 'c':
    case KEY_Esc:
        m_state = STATE_EMPTY;
        PrintString();
        break;
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

void MainWindow::ProcessDigit(uint16_t digit)
{
    int32_t value;

    if(m_state == STATE_EMPTY)
    {
        value = 0;
        m_state = STATE_VAL1;
    }
    else if(m_state == STATE_VAL1)
    {
        value = m_value1;
    }
    else if(m_state == STATE_ACTION)
    {
        value = 0;
        m_state = STATE_VAL2;
    }
    else if(m_state == STATE_VAL2)
    {
        value = m_value2;
    }

    value = value*10 + digit;

    if(m_state == STATE_VAL1)
    {
        m_value1 = value;
    }
    else if(m_state == STATE_VAL2)
    {
        m_value2 = value;
    }

    PrintString();
}

void MainWindow::RememberAction(uint16_t action)
{
    if(m_state == STATE_VAL1)
    {
        m_action = action;
        m_state = STATE_ACTION;
        PrintString();
    }
    else if(m_state == STATE_EQ)
    {
        m_value1 = m_res;
        m_action = action;
        m_state = STATE_ACTION;
        PrintString();
    }
}

void MainWindow::Compute()
{
    switch(m_action)
    {
    case ACTION_ADD:
        m_res = m_value1 + m_value2;
        break;
    case ACTION_SUBTRACT:
        m_res = m_value1 - m_value2;
        break;
    case ACTION_MULTIPLY:
        m_res = m_value1 * m_value2;
        break;
    case ACTION_DIVIDE:
        m_res = m_value1 / m_value2;
        break;
    default:
        m_res = 0;
        break;
    }

    m_state = STATE_EQ;
    PrintString();
}

void MainWindow::PrintString()
{
    char tmp[40];

    strcpy(m_string, "");

    if(m_state >= STATE_VAL1)
    {
        sprintf(tmp,"%d",m_value1);
        strcpy(m_string, tmp);
    }

    if(m_state >= STATE_ACTION)
    {
        switch(m_action)
        {
        case ACTION_ADD:
            strcat(m_string, " + ");
            break;
        case ACTION_SUBTRACT:
            strcat(m_string, " - ");
            break;
        case ACTION_MULTIPLY:
            strcat(m_string, " * ");
            break;
        case ACTION_DIVIDE:
            strcat(m_string, " / ");
            break;
        default:
            strcat(m_string, " ??? ");
            break;
        }
    }

    if(m_state >= STATE_VAL2)
    {
        sprintf(tmp,"%d",m_value2);
        strcat(m_string, tmp);
    }

    if(m_state >= STATE_EQ)
    {
        sprintf(tmp," = %d",m_res);
        strcat(m_string, tmp);
    }

    m_pText->SetText(m_string);
    ReDraw();
}

// функция main
int main(int argc, char **argv)
{
    MainWindow *pWindow = new MainWindow;

    int res = Run(argc, argv, pWindow, 370, 340);

    delete pWindow;

    return res;
}
