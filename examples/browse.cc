#include <iostream>
#include <cstring>
#include <cassert>
#include <syscall.h>
#include <fcntl.h> /* флажки чтения и записи */
#include <sys/types.h> /* определения типов */
#include <sys/stat.h> /* структура, возвращаемая stat */
#include "dirent.h"
#include <unistd.h>

#include "window.h"
#include "image.h"
#include "text.h"
#include "list.h"
#include "scroll.h"
#include "GUI.h"

#define WIN_WIDTH   800
#define WIN_HEIGHT  500
#define PATH_HEIGHT 20
#define LISTSIZE    200
#define BARSIZE     12
#define MAX_PATH    1024
#define START_PREVIEW_SIZE  120
#define GAP         10


class Table;

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

    void CreateList(const char *);
    void DisplayImage(IMAGEINFO ii);

private:
    Text *m_pText;
    List *m_pList;
    Table *m_pTable;
    Scroll *m_pListScroll, *m_pTableScroll;
    Image *m_pImage;
    char m_path[MAX_PATH];
    char **m_names;
    uint16_t m_n;
    bool m_bMode;       // true - отображается крупная картинка, false - отображаются панели списка и образцов
};

class Preview :public Window
{
public:
    Preview(const char *filename, MainWindow *pMainWindow);
    ~Preview();

    void OnCreate();
    bool OnLeftMouseButtonDoubleClick(const Point &position);
    void SetPreviewSize(uint16_t nPreviewSize);

private:
    const char  *m_filename;
    IMAGEINFO m_ii;
    Image *m_pImage;
    Text  *m_pText;
    MainWindow *m_pMainWindow;
};

class Table : public Window
{
public:
    Table(List *pFileList, MainWindow *pMainWindow);
    ~Table();

    void OnDraw(Context *cr);
    void OnSizeChanged();
    Rect &GetDataRect();

    void Update();      // обновление списка файлов
    uint16_t GetPreviewSize();
    void SetPreviewSize(uint16_t nPreviewSize);

private:
    void Reposition();  // назначение положения образцов
    void ClearPreviews(); // удаление всех образцов

    uint16_t m_nPreviewSize;
    uint16_t m_nPreviews;
    List *m_pFileList;
    MainWindow *m_pMainWindow;
    Preview **m_pPreviews;
    Rect  m_TableSize;
};

MainWindow::MainWindow()
{
    m_ClassName = __FUNCTION__;
    m_names = nullptr;
    m_n = 0;
    SetBackColor(RGB_WHITE);
    m_pList = nullptr;
    m_pTable = nullptr;
    m_bMode = false;
}

MainWindow::~MainWindow()
{
    if(m_n>0)
    {
        for(uint16_t i=0; i< m_n; i++)
        {
            free(m_names[i]);
        }
        free(m_names);
        m_names = nullptr;
        m_n = 0;
    }
}

void MainWindow::OnCreate()
{
	Rect mysize = GetInteriorSize();
	uint16_t w = mysize.GetWidth();
	uint16_t h = mysize.GetHeight();

	// текущий каталог
	m_pText = new Text;
	m_pText->SetFrameWidth(1);
    m_pText->SetAlignment(TEXT_ALIGNH_LEFT|TEXT_ALIGNV_CENTER);
	AddChild(m_pText, Point(0,0), Rect(w,PATH_HEIGHT));

	// панель выбора каталога
	m_pListScroll = new Scroll;
	AddChild(m_pListScroll, Point(0,PATH_HEIGHT), Rect(LISTSIZE,h-PATH_HEIGHT));
	m_pList = new List();
    m_pListScroll->SetDataWindow(m_pList);

    // панель превью
	m_pTableScroll = new Scroll;
	AddChild(m_pTableScroll, Point(LISTSIZE+BARSIZE,PATH_HEIGHT), Rect(w-(LISTSIZE+BARSIZE),h-PATH_HEIGHT));
	m_pTable = new Table(m_pList,this);
    m_pTableScroll->SetDataWindow(m_pTable);

    // крупная картинка
    m_pImage = new Image;
    m_pImage->Hide();
    m_pImage->SetStyle(IMAGE_SCALE_FIT|IMAGE_ALIGNH_CENTER|IMAGE_ALIGNV_CENTER);
    AddChild(m_pImage, Point(0,0), mysize);

    CreateList(".");

    CaptureKeyboard(this);
}

void MainWindow::OnDraw(Context *cr)
{
    Rect is = GetInteriorSize();
    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(LISTSIZE,0),Rect(BARSIZE,is.GetHeight()));
}

void MainWindow::OnSizeChanged()
{
	Rect mysize = GetInteriorSize();
	uint16_t w = mysize.GetWidth();
	uint16_t h = mysize.GetHeight();

	m_pText->SetPosition(Point(0,0));
	m_pText->SetSize(Rect(w,PATH_HEIGHT));

	m_pListScroll->SetPosition(Point(0,PATH_HEIGHT));
	m_pListScroll->SetSize(Rect(LISTSIZE,h-PATH_HEIGHT));

	assert(w > LISTSIZE+BARSIZE);
	m_pTableScroll->SetPosition(Point(LISTSIZE+BARSIZE,PATH_HEIGHT));
	m_pTableScroll->SetSize(Rect(w-(LISTSIZE+BARSIZE),h-PATH_HEIGHT));

	m_pImage->SetSize(mysize);
}

bool MainWindow::OnKeyPress(uint64_t keyval)
{
    switch(keyval)
    {
    case 'q':
        DeleteMe();
        return true;
    case '+':
    case '=':
        {
            uint16_t s = min(m_pTable->GetPreviewSize()+20, 500);
            m_pTable->SetPreviewSize(s);
            ReDraw();
        }
        return true;
    case '-':
    case '_':
        {
            uint16_t s = max(m_pTable->GetPreviewSize()-20, 60);
            m_pTable->SetPreviewSize(s);
            ReDraw();
        }
        return true;
    case KEY_Esc:
        m_pImage->Hide();
        m_pText->Show();
        m_pListScroll->Show();
        m_pTableScroll->Show();
        m_bMode = false;
        ReDraw();
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

void MainWindow::OnNotify(Window *child, uint32_t type, const Point &position)
{
    if(child == m_pListScroll && type == EVENT_LIST_LMB_CLICK)
    {
//std::cout << "List element " << m_pList->GetSelection() << " was clicked" << std::endl;
    }
    if(child == m_pListScroll && type == EVENT_LIST_LMB_DOUBLECLICK)
    {
//std::cout << "List element " << m_pList->GetSelection() << " was double clicked" << std::endl;

        int16_t i = m_pList->GetSelection();
        if(m_pList->GetValue(i))
        {
            char name[MAX_PATH];
            if (strlen(m_path)+strlen(m_names[i]) + 2 > sizeof(name))
            {
                std::cerr << "Name " << m_path << "/" << m_names[i] << "is too long" << std::endl;
            }
            else
            {
                chdir(m_names[i]);
                CreateList(".");
            }
            CaptureKeyboard(this);
        }
    }
}

void MainWindow::DisplayImage(IMAGEINFO ii)
{
    m_pImage->SetImage(ii);
    m_pImage->Show();
    m_pText->Hide();
    m_pListScroll->Hide();
    m_pTableScroll->Hide();
    m_bMode = true;
    ReDraw();
}

Table::Table(List *pFileList, MainWindow *pMainWindow)
{
    m_ClassName = __FUNCTION__;
    m_pFileList = pFileList;
    m_pMainWindow = pMainWindow;
    m_nPreviewSize = START_PREVIEW_SIZE;
    m_nPreviews = 0;
    m_pPreviews = nullptr;
    m_TableSize = Rect(0,0);
}

Table::~Table()
{
    if(m_nPreviews>0)
    {
        free(m_pPreviews);
        m_pPreviews = nullptr;
        m_nPreviews = 0;
    }

    SetOrigin(Point(0,0));
}

void Table::OnDraw(Context *cr)
{
    cr->SetColor(m_backColor);
    cr->FillRectangle(Point(0,0),m_TableSize);
}

void Table::OnSizeChanged()
{
    Reposition();
}

void Table::Update()
{
    ClearPreviews();

    uint16_t n = m_pFileList->GetNumberOfElements();
    for(uint16_t i=0; i<n; i++)
    {
        // очередной элемент
        Window *pElement = m_pFileList->GetElement(i);

        // пропускаем каталоги
        if(!m_pFileList->GetValue(i))
        {
            // текст
            Text *pText = reinterpret_cast<Text *>(pElement);
            char *filename = pText->GetText();

            // откроем файл
            int file = open(filename, O_RDONLY);

            // прочитаем сигнатуру и закроем файл
            unsigned char sign[8];
            read(file,&sign,8);
            close(file);

            // определяем тип файла
            if((sign[0]==0x89) && (sign[1]==0x50) && (sign[2]==0x4e) && (sign[3]==0x47)
                 && (sign[4]==0x0d) && (sign[5]==0x0a) && (sign[6]==0x1a) && (sign[7]==0x0a))
            {
                // png
                Preview *pPreview = new Preview(filename,m_pMainWindow);
                AddChild(pPreview,Point (50,50), Rect(m_nPreviewSize, m_nPreviewSize));

                // расширим массив адресов образцов и занесем туда адрес только что сзданного
                if(m_nPreviews)
                {
                    m_pPreviews = (Preview **) realloc(m_pPreviews,(m_nPreviews+1)*sizeof(Preview *));
                }
                else
                {
                    m_pPreviews = (Preview **) malloc(sizeof(Preview *));
                }
                m_pPreviews[m_nPreviews++] = pPreview;
            }
        }
    }

    // расставим образцы по своим местам
    Reposition();
}

void Table::Reposition()
{
    Rect is = GetInteriorSize();
    uint16_t x = GAP, y = GAP, k = 0, w = is.GetWidth();

    for(uint16_t i=0; i<m_nPreviews; i++)
    {
        // строка заполнена?
        if(x+m_nPreviewSize > w && k > 0)
        {
            // переход на новую строку таблицы
            x = GAP;
            y += m_nPreviewSize + GAP;
            k = 0;
        }

        // положение образца
        m_pPreviews[i]->SetPosition(Point(x, y));
        x += m_nPreviewSize + GAP;
        ++k;
    }

    if(k>0)
    {
        y += m_nPreviewSize + GAP;
    }
    y = max(is.GetHeight(),y);
    w = max(w,m_nPreviewSize + 2*GAP);

    // размер документа для прокрутки
    if((m_TableSize.GetWidth() != w) || (m_TableSize.GetHeight() != y))
    {
        m_TableSize = Rect(w,y);
        GetParent()->OnDataRectChanged(this,m_TableSize);
    }
}

void Table::ClearPreviews()
{
    if(m_nPreviews>0)
    {
        for(uint16_t i=0; i<m_nPreviews; i++)
        {
            DeleteChild(m_pPreviews[i]);
        }

        free(m_pPreviews);
        m_pPreviews = nullptr;
        m_nPreviews = 0;
    }

    SetOrigin(Point(0,0));
}

Rect &Table::GetDataRect()
{
    return m_TableSize;
}

uint16_t Table::GetPreviewSize()
{
    return m_nPreviewSize;
}

void Table::SetPreviewSize(uint16_t nPreviewSize)
{
    m_nPreviewSize = nPreviewSize;

    for(uint16_t i=0; i<m_nPreviews; i++)
    {
        m_pPreviews[i]->SetPreviewSize(nPreviewSize);
    }

    Reposition();
}

Preview::Preview(const char *filename, MainWindow *pMainWindow)
{
    m_ClassName = __FUNCTION__;
    m_filename = filename;
    m_pMainWindow = pMainWindow;
    SetFrameWidth(1);
}

Preview::~Preview()
{
    theGUI->DeletePNG(m_ii);
}

void Preview::OnCreate()
{
    m_ii = theGUI->LoadPNG(m_filename);
    Rect is = GetInteriorSize();
    uint16_t i_h = is.GetHeight()-PATH_HEIGHT;

    m_pImage = new Image(m_ii);
    AddChild(m_pImage, Point(0,0), Rect(is.GetWidth(),i_h));
    m_pText = new Text(m_filename);
    AddChild(m_pText,Point(0,i_h), Rect(is.GetWidth(),PATH_HEIGHT));

    uint8_t style;
    if(m_ii->height < i_h && m_ii->width < is.GetWidth())
    {
        style = IMAGE_SCALE_1_1|IMAGE_ALIGNH_CENTER|IMAGE_ALIGNV_CENTER;
    }
    else
    {
        style = IMAGE_SCALE_FIT|IMAGE_ALIGNH_CENTER|IMAGE_ALIGNV_CENTER;
    }
    m_pImage->SetStyle(style);
}

bool Preview::OnLeftMouseButtonDoubleClick(const Point &position)
{
    m_pMainWindow->DisplayImage(m_ii);
    return true;
}

void Preview::SetPreviewSize(uint16_t nPreviewSize)
{
    SetSize(Rect(nPreviewSize,nPreviewSize));
    Rect is = GetInteriorSize();
    uint16_t i_h = is.GetHeight()-PATH_HEIGHT;

    m_pImage->SetSize(Rect(is.GetWidth(),i_h));
    m_pText->SetSize(Rect(is.GetWidth(),PATH_HEIGHT));
    m_pText->SetPosition(Point(0,i_h));

    uint8_t style;
    if(m_ii->height < i_h && m_ii->width < is.GetWidth())
    {
        style = IMAGE_SCALE_1_1|IMAGE_ALIGNH_CENTER|IMAGE_ALIGNV_CENTER;
    }
    else
    {
        style = IMAGE_SCALE_FIT|IMAGE_ALIGNH_CENTER|IMAGE_ALIGNV_CENTER;
    }
    m_pImage->SetStyle(style);
}

int main(int argc, char **argv)
{
    MainWindow *pWindow = new MainWindow;
//    pWindow->m_filename = argc>0 ? argv[1] : nullptr;

    int res = Run(argc, argv, pWindow, WIN_WIDTH, WIN_HEIGHT);

    delete pWindow;

    return res;
}

void mysort(char **strings, uint16_t n);
bool gt(char *s1, char *s2);
uint32_t GetSymbolUTF8(char *s, uint16_t *p);

void MainWindow::CreateList(const char * dir)
{
    chdir(dir);
    strcpy(m_path, get_current_dir_name());
    m_pText->SetText(m_path);
    m_pList->Clear();

    if(m_n>0)
    {
        for(uint16_t i=0; i< m_n; i++)
        {
            free(m_names[i]);
        }
        free(m_names);
        m_names = nullptr;
        m_n = 0;
    }

    // см. dirwalk() в книге Керниган, Ричи "Язык программирования Си", раздел 8.6
    char name[MAX_PATH];
    struct dirent *dp;
    DIR *dfd;

    if ((dfd = opendir(m_path)) == NULL)
    {
        std::cerr << "Cannot open " << dir << std::endl;;
        return;
    }
    while ((dp = readdir(dfd)) != NULL)
    {
        if(!strcmp(dp->d_name, "."))
        {
            continue;
        }
        if (strlen(dir)+strlen(dp->d_name) + 2 > sizeof(name))
        {
            std::cerr << "Name " << dir << "/" << dp->d_name << "is too long" << std::endl;
        }
        else {
            if(m_n==0)
            {
                m_names = (char**) malloc(sizeof(char*));
            }
            else
            {
                m_names = (char**) realloc(m_names, (m_n+1)*sizeof(char*));
            }
            m_names[m_n] = (char*) malloc(strlen(dp->d_name)+1);
            strcpy(m_names[m_n],dp->d_name);
            ++m_n;
        }
    }
    closedir(dfd);

    // сортировка
    mysort(m_names, m_n);

    // добавляю в список
    for(uint16_t i=0; i<m_n; i++)
    {
        Text *pText = new Text(m_names[i]);
        struct stat stbuf;
        void *value = 0;
        char name[MAX_PATH];
        if (strlen(m_path)+strlen(m_names[i]) + 2 > sizeof(name))
        {
            std::cerr << "Name " << m_path << "/" << m_names[i] << "is too long" << std::endl;
        }
        else
        {
            strcpy(name,m_path);
            strcat(name,"/");
            strcat(name,m_names[i]);

            if(stat(name, &stbuf) != -1 && (stbuf.st_mode & S_IFMT) == S_IFDIR)
            {
                value = (void *)1;
                pText->SetFont(NULL, -1, TEXT_STYLE_BOLD, -1);
            }
        }
        m_pList->Insert(m_n, pText, value);
    }

    // обновим образцы
    m_pTable->Update();
}

// сортировка массива строк
void mysort(char **strings, uint16_t n)
{
    for(uint16_t i=0; i<n; i++)
    {
        for(uint16_t j=0; j<n-i-1; j++)
        {
            if(gt(strings[j],strings[j+1]))
            {
                char *tmp = strings[j];
                strings[j] = strings[j+1];
                strings[j+1] = tmp;
            }
        }
    }
}

// сравнение двух строк UTF8
bool gt(char *s1, char *s2)
{
    uint16_t p1 = 0, p2 = 0;

    // пока строки не закончились
    while(s1[p1] && s2[p2])
    {
        // достаем символы
        uint32_t c1 = GetSymbolUTF8(s1,&p1);
        uint32_t c2 = GetSymbolUTF8(s2,&p2);

        // сравниваем
        if(c1 > c2)
        {
            return true;
        }
        else if(c1 < c2)
        {
            return false;
        }
    }

    // какая строка длиннее - та и больше
    if(s1[p1])
    {
        return true;
    }
    else if(s2[p2])
    {
        return false;
    }

    // строки равны
    return false;
}

// чтение символа UTF8 из строки
uint32_t GetSymbolUTF8(char *s, uint16_t *p)
{
    uint32_t res = 0, tmp = 0;

    if((s[*p] & 0x80) == 0x00)
    {
        // UTF-8: ASCII (8 bits)
        res = s[*p];
        ++*p;
    }
    else if((s[*p] & 0xe0) == 0xc0)
    {
        // UTF-8: 16 bits
        res = s[*p] & 0x1f;
        res <<= 6;
        res |= s[*p+1] & 0x3f;
        *p += 2;
    }
    else if((s[*p] & 0xf0) == 0xe0)
    {
        // UTF-8: 24 bits
        res = s[*p] & 0x0f;
        res <<= 12;
        tmp = s[*p+1] & 0x3f;
        tmp <<= 6;
        res |= tmp;
        res |= s[*p+2] & 0x3f;
        *p += 3;
    }
    else if((s[*p] & 0xf8) == 0xf0)
    {
        // UTF-8: 32 bits
        res = s[*p] & 0x07;
        res <<= 18;
        tmp = s[*p+1] & 0x3f;
        tmp <<= 12;
        res |= tmp;
        tmp = s[*p+2] & 0x3f;
        tmp <<= 6;
        res |= tmp;
        res |= s[*p+3] & 0x3f;
        *p += 4;
    }

    return res;
}
