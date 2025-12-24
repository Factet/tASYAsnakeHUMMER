#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstdio>

#include "window.h"
#include "text.h"
#include "image.h"
#include "button.h"
#include "GUI.h"

// -------------------------------------------------
//  ПАРАМЕТРЫ ПОЛЯ И ОКНА
// -------------------------------------------------

// Количество клеток по горизонтали и вертикали
#define GRID_W  15
#define GRID_H  15

// Размер одной клетки в пикселях
#define CELL    40

// Размеры поля в пикселях
#define FIELD_W (GRID_W * CELL)
#define FIELD_H (GRID_H * CELL)

// Размер всего окна
#define WIN_W   700
#define WIN_H   800

// Скорость передвижения змейки (чем больше значение — тем медленнее двигается змейка)
#define TIMER_MS 150

// Опрелеляем значение пи, для будущей отрисовки клубнички
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -------------------------------------------------
//  ЦВЕТА
// -------------------------------------------------

// Всё окошко
static const RGB COL_WINDOW_BG   = RGB(0.7, 0.7, 0.7);

// Цвета клеток по аналогу шахматной доски
static const RGB COL_FIELD_LIGHT = RGB(0.80, 0.92, 0.80);
static const RGB COL_FIELD_DARK  = RGB(0.60, 0.85, 0.60);

// Рамки вокруг поля
static const RGB COL_FIELD_FRAME = RGB(0.0, 0.0, 0.0);

// Тело и голова змейки
static const RGB COL_SNAKE_BODY  = RGB(0.35, 0.70, 1.00);
static const RGB COL_SNAKE_HEAD  = RGB(0.10, 0.45, 0.90);

// Цвет клубнички
static const RGB COL_FOOD        = RGB(0.90, 0.20, 0.20);

// -------------------------------------------------
//  НАПРАВЛЕНИЕ ЗМЕЙКИ
// -------------------------------------------------

// Четыре возможных направления движения
enum Direction
{
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

// -------------------------------------------------
//  СОБЫТИЯ КНОПОК
// -------------------------------------------------

enum UserEventType
{
    EVENT_START = 1,   // нажали «СТАРТ»
    EVENT_RESTART      // нажали «Сыграть ещё»
};

// -------------------------------------------------
//  КЛАСС ГЛАВНОГО ОКНА
// -------------------------------------------------

class MainWindow : public Window
{
public:
    MainWindow();
    ~MainWindow() {}

    // Старт жизни окна: создаём виджеты.
    void OnCreate();

    // Перерисовка всего окна.
    void OnDraw(Context *cr);

    // Изменился размер окна (растянули/сузили)
    void OnSizeChanged();

    // Нажатие клавиши
    bool OnKeyPress(uint64_t keyval);

    // Клик ЛКМ
    bool OnLeftMouseButtonClick(const Point &position);

    // Таймер (вызывается каждые TIMER_MS миллисекунд)
    bool OnTimeout();

    // События от кнопок
    void OnNotify(Window *child, uint32_t type, const Point &position);

private:
    // Запустить новую игру (из меню или после проигрыша)
    void StartGame();

    // Обработать проигрыш
    void GameOver();

    // Поставить клубничку в случайное свободное место
    void PlaceFood();

    // Сдвинуть змейку
    void MoveSnake();

    // Проверить столкновения (стены / сам себя)
    void CheckCollision();

    // Рисование отдельных частей
    void DrawField(Context *cr);
    void DrawSnake(Context *cr);
    void DrawFood(Context *cr);
    void DrawBerry(Context *cr, int cx, int cy, int size);
    void DrawMenuBackground(Context *cr);
    void LayoutControls();

private:
    Text       *m_pTitle;      // «ЗМЕЙКА» / «ПРОИГРЫШ»
    Text       *m_pScoreText;  // «score: N»
    Text       *m_pPauseText;  // «ПАУЗА» поверх поля
    TextButton *m_pStart;      // кнопка «СТАРТ» в главном меню
    TextButton *m_pRestart;    // кнопка «Сыграть ещё» после проигрыша

    // Координаты змейки 
    int       m_snakeX[GRID_W * GRID_H];
    int       m_snakeY[GRID_W * GRID_H];
    int       m_snakeLen;      // текущая длина змейки
    Direction m_dir;           // текущее направление головы

    bool      m_gameOver;      // сейчас проигрыш?
    bool      m_paused;        // сейчас пауза?
    bool      m_inMenu;        // сейчас стартовое меню?

    int       m_foodX;         // координата клубнички (клетка X)
    int       m_foodY;         // координата клубнички (клетка Y)

    int       m_score;         // счёт

    // Параметры анимации «тряски» при проигрыше
    bool      m_shake;
    int       m_shakeTicks;    // сколько тиков уже трясём
    int       m_shakeMaxTicks; // сколько всего будем трясти
    int       m_shakeDX;       // горизонтальный сдвиг поля (в пикселях)
    int       m_shakeDY;       // (зарезервировано, сейчас 0)

    // Позиция поля внутри окна (левый верхний угол в пикселях)
    int       m_fieldLeft;
    int       m_fieldTop;
};

// -------------------------------------------------
//  РЕАЛИЗАЦИЯ MainWindow
// -------------------------------------------------

MainWindow::MainWindow()
{
    m_ClassName = __FUNCTION__;

    // Начальные состояния
    m_gameOver  = false;
    m_paused    = false;
    m_score     = 0;
    m_inMenu    = true;    // сначала показываем главное меню

    // Параметры тряски
    m_shake         = false;
    m_shakeTicks    = 0;
    m_shakeMaxTicks = 0;
    m_shakeDX       = 0;
    m_shakeDY       = 0;

    // Поле потом центрируем в LayoutControls
    m_fieldLeft = 0;
    m_fieldTop  = 0;
}

void MainWindow::OnCreate()
{
    // Фоновый цвет всего окна
    SetBackColor(COL_WINDOW_BG);

    // --- Заголовок ---
    m_pTitle = new Text("ЗМЕЙКА");
    m_pTitle->SetFont(0, 32, TEXT_STYLE_BOLD, -1);
    m_pTitle->SetAlignment(TEXT_ALIGNH_CENTER | TEXT_ALIGNV_CENTER);
    AddChild(m_pTitle, Point(0, 0), Rect(WIN_W, 40));

    // --- Score ---
    m_pScoreText = new Text("score: 0");
    m_pScoreText->SetFont(0, 20, -1, -1);
    m_pScoreText->SetAlignment(TEXT_ALIGNH_CENTER | TEXT_ALIGNV_TOP);
    AddChild(m_pScoreText, Point(0, 0), Rect(WIN_W, 30));
    m_pScoreText->Hide();   // в меню счёт не нужен

    // --- Пауза ---
    m_pPauseText = new Text("ПАУЗА");
    m_pPauseText->SetFont(0, 28, TEXT_STYLE_BOLD, -1);
    m_pPauseText->SetAlignment(TEXT_ALIGNH_CENTER | TEXT_ALIGNV_CENTER);
    AddChild(m_pPauseText, Point(0, 0), Rect(FIELD_W, 60));
    m_pPauseText->Hide();

    // --- Кнопка СТАРТ (главное меню) ---
    m_pStart = new TextButton("СТАРТ", EVENT_START);
    AddChild(m_pStart, Point(0, 0), Rect(220, 80));

    // --- Кнопка «Сыграть ещё» (после проигрыша) ---
    m_pRestart = new TextButton("Сыграть ещё", EVENT_RESTART);
    AddChild(m_pRestart, Point(0, 0), Rect(200, 40));
    m_pRestart->Hide();

    
    CreateTimeout(this, TIMER_MS);

    LayoutControls();
    CaptureKeyboard(this);  // сразу захватываем клавиатуру
}

void MainWindow::OnSizeChanged()
{
    // При изменении размера окна перерасположить элементы
    LayoutControls();
}

// Расстановка элементов по экрану.
void MainWindow::LayoutControls()
{
    Rect is = GetInteriorSize();
    int winW = is.GetWidth();
    int winH = is.GetHeight();

    if (m_inMenu)
    {
        // --------- РЕЖИМ МЕНЮ ---------

        int titleH = 50;
        int titleY = winH / 2 - 100;
        if (titleY < 20) titleY = 20;
        m_pTitle->SetPosition(Point(0, titleY));
        m_pTitle->SetSize(Rect(winW, titleH));

        int startW = 220, startH = 80;
        int startX = (winW - startW) / 2;
        int startY = winH / 2 - startH / 2;
        m_pStart->SetPosition(Point(startX, startY));
        m_pStart->SetSize(Rect(startW, startH));
    }
    else
    {
        // --------- РЕЖИМ ИГРЫ ---------

        int titleH   = 40;
        int scoreH   = 30;
        int titleY   = 20;
        int scoreY   = titleY + titleH;

        // Заголовок
        m_pTitle->SetPosition(Point(0, titleY));
        m_pTitle->SetSize(Rect(winW, titleH));

        // Строка score
        m_pScoreText->SetPosition(Point(0, scoreY));
        m_pScoreText->SetSize(Rect(winW, scoreH));

        // Центрируем поле
        int minFieldTop = scoreY + scoreH + 20;      // отступ сверху
        int centeredTop = (winH - FIELD_H) / 2;      // центр
        if (centeredTop < minFieldTop) centeredTop = minFieldTop;

        m_fieldTop  = centeredTop;
        m_fieldLeft = (winW - FIELD_W) / 2;
        if (m_fieldLeft < 10) m_fieldLeft = 10;      // чуть слева отступ

        // Позиция надписи «ПАУЗА» — по центру поля.
        m_pPauseText->SetPosition(
            Point(m_fieldLeft, m_fieldTop + FIELD_H / 2 - 30));
        m_pPauseText->SetSize(Rect(FIELD_W, 60));

        // Кнопка «Сыграть ещё» под полем.
        int btnW = 200, btnH = 40;
        int btnX = (winW - btnW) / 2;
        int btnY = m_fieldTop + FIELD_H + 20;
        if (btnY + btnH > winH - 10) btnY = winH - btnH - 10;
        m_pRestart->SetPosition(Point(btnX, btnY));
        m_pRestart->SetSize(Rect(btnW, btnH));
    }
}

bool MainWindow::OnLeftMouseButtonClick(const Point &)
{
    // Забираем фокус клавиатуры обратно
    CaptureKeyboard(this);
    return true;
}

// -------------------------------------------------
//  ЛОГИКА ИГРЫ
// -------------------------------------------------

// Запустить новую игру (и из меню, и после проигрыша)
void MainWindow::StartGame()
{
    m_inMenu   = false;
    m_gameOver = false;
    m_paused   = false;
    m_score    = 0;

    // Вернули заголовок
    m_pTitle->SetText("ЗМЕЙКА");
    m_pScoreText->SetText("score: 0");
    m_pScoreText->Show();

    m_pRestart->Hide();
    m_pPauseText->Hide();
    m_pStart->Hide();

    // Сбрасываем параметры тряски
    m_shake         = false;
    m_shakeTicks    = 0;
    m_shakeMaxTicks = 0;
    m_shakeDX       = 0;
    m_shakeDY       = 0;

    // Начальная змейка
    int cx = GRID_W / 2;
    int cy = GRID_H / 2;
    m_snakeLen = 3;
    m_snakeX[0] = cx;
    m_snakeY[0] = cy;
    m_snakeX[1] = cx - 1;
    m_snakeY[1] = cy;
    m_snakeX[2] = cx - 2;
    m_snakeY[2] = cy;

    m_dir = DIR_RIGHT; 

    PlaceFood();

    LayoutControls();
    ReDraw();
}

// Обработка проигрыша
void MainWindow::GameOver()
{
    // Если уже в состоянии проигрыша — повторно не реагируем
    if (m_gameOver) return;

    m_gameOver = true;
    m_pTitle->SetText("ПРОИГРЫШ");
    m_pRestart->Show();
    m_pPauseText->Hide();   

    // Включаем тряску поля
    m_shake         = true;
    m_shakeTicks    = 0;
    m_shakeMaxTicks = 25;   
    m_shakeDX       = 0;
    m_shakeDY       = 0;

    ReDraw();
}

// Ставим клубничку в рандомное место
void MainWindow::PlaceFood()
{
    while (true)
    {
        m_foodX = std::rand() % GRID_W;
        m_foodY = std::rand() % GRID_H;

        // Проверяем, что ягода не попала на тело змейки
        bool clash = false;
        for (int i = 0; i < m_snakeLen; ++i)
        {
            if (m_snakeX[i] == m_foodX && m_snakeY[i] == m_foodY)
            {
                clash = true;
                break;
            }
        }
        if (!clash) break;
    }
}

// --- Логика движения и роста змейки ---
void MainWindow::MoveSnake()
{
    // Запоминаем старый хвост до сдвига 
    int oldTailX = m_snakeX[m_snakeLen - 1];
    int oldTailY = m_snakeY[m_snakeLen - 1];

    // Сдвигаем тело с хвоста к голове
    for (int i = m_snakeLen - 1; i > 0; --i)
    {
        m_snakeX[i] = m_snakeX[i - 1];
        m_snakeY[i] = m_snakeY[i - 1];
    }

    // Двигаем голову в зависимости от направления
    switch (m_dir)
    {
        case DIR_UP:    m_snakeY[0]--; break;
        case DIR_DOWN:  m_snakeY[0]++; break;
        case DIR_LEFT:  m_snakeX[0]--; break;
        case DIR_RIGHT: m_snakeX[0]++; break;
    }

    // Если голова попала в клубничку —
    // увеличиваем длину и ставим новую ягоду
    if (m_snakeX[0] == m_foodX && m_snakeY[0] == m_foodY)
    {
        if (m_snakeLen < GRID_W * GRID_H)
        {
            m_snakeX[m_snakeLen] = oldTailX;
            m_snakeY[m_snakeLen] = oldTailY;
            m_snakeLen++;
        }

        m_score += 10;
        char buf[64];
        std::sprintf(buf, "score: %d", m_score);
        m_pScoreText->SetText(buf);

        PlaceFood();
    }
}

// Проверка столкновений
void MainWindow::CheckCollision()
{
    // Столкновение со стеной
    if (m_snakeX[0] < 0 || m_snakeX[0] >= GRID_W ||
        m_snakeY[0] < 0 || m_snakeY[0] >= GRID_H)
    {
        GameOver();
        return;
    }

    // Столкновение с самим собой
    for (int i = 1; i < m_snakeLen; ++i)
    {
        if (m_snakeX[i] == m_snakeX[0] &&
            m_snakeY[i] == m_snakeY[0])
        {
            GameOver();
            return;
        }
    }
}

// -------------------------------------------------
//  СОБЫТИЯ
// -------------------------------------------------

bool MainWindow::OnKeyPress(uint64_t keyval)
{
    uint32_t key = (uint32_t)keyval;

    // q / Q — выход из программы.
    if (key == 'q' || key == 'Q')
    {
        DeleteMe();
        return true;
    }

    // В меню только реагируем на Enter/Space для старта.
    if (m_inMenu)
    {
        if (key == KEY_Return || key == ' ')
        {
            StartGame();
        }
        return true;
    }

    // Переключение паузы по P/p, если игра идёт.
    if ((key == 'p' || key == 'P') && !m_gameOver)
    {
        m_paused = !m_paused;
        if (m_paused) m_pPauseText->Show();
        else          m_pPauseText->Hide();
        ReDraw();
        return true;
    }

    // Управление стрелками, только если не пауза и не проигрыш.
    if (!m_gameOver && !m_paused)
    {
        switch (key)
        {
            case KEY_Up:
                if (m_dir != DIR_DOWN)  m_dir = DIR_UP;
                break;
            case KEY_Down:
                if (m_dir != DIR_UP)    m_dir = DIR_DOWN;
                break;
            case KEY_Left:
                if (m_dir != DIR_RIGHT) m_dir = DIR_LEFT;
                break;
            case KEY_Right:
                if (m_dir != DIR_LEFT)  m_dir = DIR_RIGHT;
                break;
            default:
                ;
        }
    }

    return true;
}

// Таймер
bool MainWindow::OnTimeout()
{
    // Если игра активна — двигаем змейку
    if (!m_inMenu && !m_gameOver && !m_paused)
    {
        MoveSnake();
        CheckCollision();
        ReDraw();
    }
    // Если проигрыш и включена тряска — анимируем её.
    else if (!m_inMenu && m_gameOver && m_shake)
    {
        m_shakeTicks++;

        // Немного двигаем поле вправо-влево.
        m_shakeDX = (std::rand() % 7) - 3;  // −3..+3
        m_shakeDY = 0;

        if (m_shakeTicks >= m_shakeMaxTicks)
        {
            // Заканчиваем тряску
            m_shake = false;
            m_shakeDX = 0;
            m_shakeDY = 0;
        }
        ReDraw();
    }
    return true; // 
}

// События от кнопок
void MainWindow::OnNotify(Window *child, uint32_t type, const Point &)
{
    if (child == m_pStart && type == EVENT_START)
    {
        StartGame();
    }
    else if (child == m_pRestart && type == EVENT_RESTART)
    {
        StartGame();
    }
}

// -------------------------------------------------
//  ОТРИСОВКА
// -------------------------------------------------

// Фон меню
void MainWindow::DrawMenuBackground(Context *cr)
{
    Rect is = GetInteriorSize();
    cr->SetColor(COL_WINDOW_BG);
    cr->FillRectangle(Point(0, 0), is);
}

// Рисование поля
void MainWindow::DrawField(Context *cr)
{
    Rect is = GetInteriorSize();
    (void)is;

    int baseLeft = m_fieldLeft + m_shakeDX;
    int baseTop  = m_fieldTop;

    // Рамка вокруг поля
    cr->SetColor(COL_FIELD_FRAME);
    cr->FillRectangle(Point(baseLeft - 2, baseTop - 2),
                      Rect(FIELD_W + 4, 2));
    cr->FillRectangle(Point(baseLeft - 2, baseTop + FIELD_H),
                      Rect(FIELD_W + 4, 2));
    cr->FillRectangle(Point(baseLeft - 2, baseTop - 2),
                      Rect(2, FIELD_H + 4));
    cr->FillRectangle(Point(baseLeft + FIELD_W, baseTop - 2),
                      Rect(2, FIELD_H + 4));

    // Заполняем поле клетками в шахматном порядке
    for (int y = 0; y < GRID_H; ++y)
    {
        for (int x = 0; x < GRID_W; ++x)
        {
            RGB col = ((x + y) % 2 == 0) ? COL_FIELD_LIGHT : COL_FIELD_DARK;
            int px = baseLeft + x * CELL;
            int py = baseTop  + y * CELL;

            cr->SetColor(col);
            cr->FillRectangle(Point(px, py), Rect(CELL, CELL));
        }
    }
}

// --- Рисование змейки ---
void MainWindow::DrawSnake(Context *cr)
{
    int baseLeft = m_fieldLeft + m_shakeDX;
    int baseTop  = m_fieldTop;

    // Сначала тело
    for (int i = 1; i < m_snakeLen; ++i)
    {
        int px = baseLeft + m_snakeX[i] * CELL;
        int py = baseTop  + m_snakeY[i] * CELL;

        cr->SetColor(COL_SNAKE_BODY);
        cr->FillRectangle(Point(px + 1, py + 1),
                          Rect(CELL - 2, CELL - 2));
    }

    // Потом голова
    int hx = baseLeft + m_snakeX[0] * CELL;
    int hy = baseTop  + m_snakeY[0] * CELL;

    cr->SetColor(COL_SNAKE_HEAD);
    cr->FillRectangle(Point(hx + 1, hy + 1),
                      Rect(CELL - 2, CELL - 2));

    // Глаза: делаем их ближе друг к другу, по центру головы
    int eyeSize = 4;
    int margin  = 5;                       // расстояние от “переднего края”
    int midX    = hx + CELL / 2;
    int midY    = hy + CELL / 2;

    cr->SetColor(RGB(0.0, 0.0, 0.0));      // чёрные глазки-бусинки

    switch (m_dir)
    {
        case DIR_RIGHT:
        {
            // Смотрим вправо: оба глаза ближе к правому краю, по центру по вертикали
            int ex  = hx + CELL - margin - eyeSize;
            int ey1 = midY - eyeSize - 1;
            int ey2 = midY + 1;
            cr->FillRectangle(Point(ex, ey1), Rect(eyeSize, eyeSize));
            cr->FillRectangle(Point(ex, ey2), Rect(eyeSize, eyeSize));
            break;
        }
        case DIR_LEFT:
        {
            // Влево: глаза ближе к левому краю
            int ex  = hx + margin;
            int ey1 = midY - eyeSize - 1;
            int ey2 = midY + 1;
            cr->FillRectangle(Point(ex, ey1), Rect(eyeSize, eyeSize));
            cr->FillRectangle(Point(ex, ey2), Rect(eyeSize, eyeSize));
            break;
        }
        case DIR_UP:
        {
            // Вверх: глаза ближе к верхнему краю, но не по углам, а ближе к центру
            int ey  = hy + margin;
            int ex1 = midX - eyeSize - 1;
            int ex2 = midX + 1;
            cr->FillRectangle(Point(ex1, ey), Rect(eyeSize, eyeSize));
            cr->FillRectangle(Point(ex2, ey), Rect(eyeSize, eyeSize));
            break;
        }
        case DIR_DOWN:
        {
            // Вниз: аналогично, но у нижнего края
            int ey  = hy + CELL - margin - eyeSize;
            int ex1 = midX - eyeSize - 1;
            int ex2 = midX + 1;
            cr->FillRectangle(Point(ex1, ey), Rect(eyeSize, eyeSize));
            cr->FillRectangle(Point(ex2, ey), Rect(eyeSize, eyeSize));
            break;
        }
    }
}

// Рисование клубнички
void MainWindow::DrawBerry(Context *cr, int cx, int cy, int size)
{
    // Немного другой “профиль”: сверху уже и более плоско,
    // снизу шире и чуть вытянутей – получается более клубничная форма.
    int rx = size / 2 - 4;
    int ry = size / 2;

    const int N = 32;    // чуть больше точек — контур глаже
    Point pts[N];

    for (int i = 0; i < N; ++i)
    {
        double angle = 2.0 * M_PI * i / N;
        double ca = std::cos(angle);
        double sa = std::sin(angle);

        double x, y;

        if (sa < 0)
        {
            // Верхняя часть — сужаем по X и сильно сплющиваем по Y
            x = cx + rx * ca * 0.8;
            y = cy + ry * sa * 0.35;
        }
        else
        {
            // Нижняя часть — делаем её чуть шире и ниже
            x = cx + rx * ca * 1.05;
            y = cy + ry * sa * 1.15;
        }

        pts[i] = Point((int)x, (int)y);
    }

    // Красное тело ягоды
    cr->SetColor(COL_FOOD);
    cr->FillPolyline(N, pts);

    // Зелёные листики (чуть крупнее)
    int topY = cy - ry * 0.5;  // из-за сплющивания вершина выше
    Point leaf[3];
    leaf[0] = Point(cx - 7, topY + 3);
    leaf[1] = Point(cx + 7, topY + 3);
    leaf[2] = Point(cx,     topY - 6);
    cr->SetColor(RGB(0.0, 0.6, 0.0));
    cr->FillPolyline(3, leaf);

    // Ножка
    cr->FillRectangle(Point(cx - 1, topY - 8), Rect(2, 5));

    // Жёлтые семечки: распределяем по “нижней половине”
    cr->SetColor(RGB(1.0, 0.95, 0.6));
    for (int i = -2; i <= 2; ++i)
    {
        int sx1 = cx + i * 4;
        int sy1 = cy + 3;
        int sy2 = cy + 8;
        cr->FillRectangle(Point(sx1, sy1), Rect(2, 2));
        cr->FillRectangle(Point(sx1 + 2, sy2), Rect(2, 2));
    }
}

// Рисуем клубничку в нужной клеточке
void MainWindow::DrawFood(Context *cr)
{
    int baseLeft = m_fieldLeft + m_shakeDX;
    int baseTop  = m_fieldTop;

    int cellX = baseLeft + m_foodX * CELL;
    int cellY = baseTop  + m_foodY * CELL;

    int cx = cellX + CELL / 2;
    int cy = cellY + CELL / 2 + 2; 

    DrawBerry(cr, cx, cy, CELL);
}

// Рисование окна
void MainWindow::OnDraw(Context *cr)
{
    Rect is = GetInteriorSize();

    cr->SetColor(COL_WINDOW_BG);
    cr->FillRectangle(Point(0, 0), is);

    if (m_inMenu)
    {
        DrawMenuBackground(cr);
        return;
    }

    DrawField(cr);
    DrawSnake(cr);
    DrawFood(cr);
}

// -------------------------------------------------
//  MAIN
// -------------------------------------------------

int main(int argc, char **argv)
{
    std::srand(std::time(NULL));  

    MainWindow *p = new MainWindow;
    int res = Run(argc, argv, p, WIN_W, WIN_H);
    delete p;
    return res;
}

