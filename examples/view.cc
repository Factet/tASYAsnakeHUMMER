#include <iostream>
#include <cstring>

#include "window.h"
#include "text.h"
#include "scroll.h"
#include "GUI.h"

#define WIN_WIDTH   800
#define WIN_HEIGHT  500
#define BUFSIZE     4096

char *GetFile(const char *filename);

class MainWindow : public Window
{
public:
    MainWindow() {}
    ~MainWindow() {}

    void OnCreate();
    bool OnKeyPress(uint64_t value);
    void OnSizeChanged();

public:
    char *m_filename;

private:
    Text *m_pText;
    Scroll *m_pScroll;
};

void MainWindow::OnCreate()
{
	Rect mysize = GetInteriorSize();

	m_pScroll = new Scroll;
	AddChild(m_pScroll, Point(0,0), mysize);

	m_pText = new Text();
    m_pText->SetFont("Monospace", 14, -1, -1);
    m_pText->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_TOP);
    m_pText->SetWrap(false);
    m_pScroll->SetDataWindow(m_pText);
//    m_pScroll->SetBackColor(RGB_WHITE);
//    m_pText->SetBackColor(RGB_WHITE);
    CaptureKeyboard(this);

    char *buf = GetFile(m_filename);
    m_pText->SetText(buf);
    free(buf);
}

void MainWindow::OnSizeChanged()
{
	Rect mysize = GetInteriorSize();

	m_pScroll->SetSize(mysize);
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

int main(int argc, char **argv)
{
    MainWindow *pWindow = new MainWindow;
    pWindow->m_filename = argc>0 ? argv[1] : nullptr;

    int res = Run(argc, argv, pWindow, WIN_WIDTH, WIN_HEIGHT);

    delete pWindow;

    return res;
}

// возвращает буфер, содержащий считанный в виде строки файл
// эта функция не пригодна для считывания двоичных файлов!
char *GetFile(const char *filename)
{
    size_t bufsize = BUFSIZE;
    char *buf = (char *) malloc(bufsize);

    if(filename)
    {
        FILE *in = fopen(filename,"r");
        if(!in)
        {
            strcpy(buf, "Не удалось открыть файл ");
            strcat(buf, filename);
        }
        else
        {
            while(1)
            {
                size_t s = fread(buf+bufsize-BUFSIZE, sizeof(char), BUFSIZE, in);
                if(s < BUFSIZE)
                {
                    buf[bufsize-BUFSIZE+s] = 0;
                    fclose(in);
                    break;
                }
                else
                {
                    bufsize += BUFSIZE;
                    buf = (char *) realloc(buf, bufsize);
                }
            }
        }
    }
    else
    {
        strcpy(buf, "Не задано имя файла");
    }

    return buf;
}
