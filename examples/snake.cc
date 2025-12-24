// examples/snake.cc
#include <deque>
#include <random>
#include "window.h"
#include "text.h"
#include "button.h"
#include "GUI.h"

// ---- размеры поля/окна ----
#define CELL    20
#define GRID_W  20
#define GRID_H  15
#define WIN_W   (GRID_W*CELL)
#define WIN_H   (GRID_H*CELL)

// ---- цвета ----
#define MENU_BG        RGB(0.25, 0.65, 0.25)
#define FIELD_LIGHT    RGB(0.63, 0.85, 0.63)
#define FIELD_DARK     RGB(0.46, 0.74, 0.46)
#define SNAKE_HEAD     RGB(0.10, 0.50, 0.80)
#define SNAKE_BODY     RGB(0.55, 0.80, 0.93)
#define FOOD_RED       RGB(0.88, 0.12, 0.12)
#define FOOD_GREEN     RGB(0.10, 0.60, 0.10)

// ---- события ----
enum UserEventType {
    EVENT_START = 1001,
    EVENT_RETRY,
    EVENT_SCORE_CHANGED,
    EVENT_GAME_OVER,
    EVENT_GAME_WIN
};

enum class GameState { Menu, Playing, Paused, GameOver, Win };
enum Dir { Up, Right, Down, Left };

class Board;

// -------------------- MainWindow --------------------
class MainWindow : public Window {
public:
    MainWindow(){ m_ClassName = __FUNCTION__; }
    void OnCreate() override;
    void OnDraw(Context* cr) override;
    bool OnKeyPress(uint64_t key) override;
    void OnNotify(Window* child, uint32_t type, const Point& pos) override;

private:
    void ShowMenu();
    void StartGame();
    void ShowEnd(bool win);
    void UpdateScore(int s);

    GameState m_state{GameState::Menu};
    int       m_score{0};

    Text*       m_title{};
    Text*       m_tip{};
    Text*       m_scoreText{};
    TextButton* m_btnStart{};
    TextButton* m_btnRetry{};
    Board*      m_board{};
};

// ----------------------- Board -----------------------
class Board : public Window {
public:
    Board(){ m_ClassName = __FUNCTION__; }
    void OnDraw(Context* cr) override;
    bool OnKeyPress(uint64_t key) override;
    bool OnTimeout() override;

    void Reset();
    void Pause(bool v){ m_paused=v; }
    int  Score() const { return m_score; }

private:
    void Step();
    void SpawnFood();
    bool Occupied(int x,int y) const;
    void DrawEyes(Context* cr) const;
    void DrawFood(Context* cr) const;

    std::deque<Point> m_snake;
    Dir    m_dir{Right};
    Dir    m_next{Right};
    Point  m_food{0,0};
    int    m_score{0};
    bool   m_paused{false};
    uint16_t m_stepMs{120};
    std::mt19937 m_rng{std::random_device{}()};
};

// ==================== MainWindow ====================

void MainWindow::OnCreate() {
    SetBackColor(MENU_BG);

    m_title = new Text(L"ЗМЕЙКА");
    m_title->SetFont("DejaVu Sans", 24, -1, -1);
    m_title->SetAlignment(TEXT_ALIGNH_CENTER|TEXT_ALIGNV_CENTER);
    AddChild(m_title, Point(0, WIN_H/6), Rect(WIN_W, 40));

    m_tip = new Text(L"Управление: стрелки — движение, P — пауза, Esc — меню");
    m_tip->SetFont("DejaVu Sans", 12, -1, -1);
    m_tip->SetAlignment(TEXT_ALIGNH_CENTER|TEXT_ALIGNV_CENTER);
    AddChild(m_tip, Point(0, WIN_H - 40), Rect(WIN_W, 24));

    m_scoreText = new Text(L"Счёт: 0");
    m_scoreText->SetFont("DejaVu Sans", 12, -1, -1);
    m_scoreText->Hide();
    AddChild(m_scoreText, Point(WIN_W-120, 6), Rect(114, 18));

    m_btnStart = new TextButton("ПУСК", EVENT_START);
    AddChild(m_btnStart, Point((WIN_W-140)/2, WIN_H/2-18), Rect(140,36));

    m_btnRetry = new TextButton("СЫГРАТЬ ЕЩЁ", EVENT_RETRY);
    m_btnRetry->Hide();
    AddChild(m_btnRetry, Point((WIN_W-180)/2, WIN_H/2+24), Rect(180,36));

    m_board = new Board();
    m_board->Hide();
    AddChild(m_board, Point(0,0), Rect(WIN_W,WIN_H));

    CaptureKeyboard(this);
}

void MainWindow::OnDraw(Context* cr) {
    cr->SetColor(GetBackColor());
    cr->FillRectangle(Point(0,0), GetInteriorSize());
}

bool MainWindow::OnKeyPress(uint64_t key) {
    if (key=='q'){ DeleteMe(); return true; }
    if (key==KEY_Return && (m_state==GameState::Menu || m_state==GameState::GameOver || m_state==GameState::Win)) { StartGame(); return true; }
    if (key==KEY_Esc && (m_state==GameState::Playing || m_state==GameState::Paused)) { ShowMenu(); return true; }
    return false;
}

void MainWindow::OnNotify(Window* child, uint32_t type, const Point&) {
    if (child==m_btnStart && type==EVENT_START) { StartGame(); return; }
    if (child==m_btnRetry && type==EVENT_RETRY) { StartGame(); return; }
    if (child==m_board) {
        if (type==EVENT_SCORE_CHANGED) UpdateScore(m_board->Score());
        else if (type==EVENT_GAME_OVER) ShowEnd(false);
        else if (type==EVENT_GAME_WIN)  ShowEnd(true);
    }
}

void MainWindow::ShowMenu() {
    m_state = GameState::Menu;
    SetBackColor(MENU_BG);
    m_title->SetText(L"ЗМЕЙКА");
    m_title->Show();
    m_tip->SetText(L"Управление: стрелки — движение, P — пауза, Esc — меню");
    m_tip->Show();
    m_btnStart->Show();
    m_btnRetry->Hide();
    m_scoreText->Hide();
    m_board->Hide();
    CaptureKeyboard(this);
    ReDraw();
}

void MainWindow::StartGame() {
    m_state = GameState::Playing;
    m_score = 0;
    m_scoreText->SetText(L"Счёт: 0");
    m_title->Hide(); m_tip->Hide();
    m_btnStart->Hide(); m_btnRetry->Hide();
    m_scoreText->Show();
    m_board->Show();
    m_board->Reset();
    CaptureKeyboard(m_board);
    ReDraw();
}

void MainWindow::ShowEnd(bool win) {
    m_state = win ? GameState::Win : GameState::GameOver;
    SetBackColor(MENU_BG);
    m_board->Hide();
    m_title->SetText(win ? L"ПОБЕДА!" : L"ИГРА ОКОНЧЕНА");
    m_title->Show();
    m_tip->SetText(L"Enter — сыграть ещё, Esc — меню");
    m_tip->Show();
    m_btnRetry->Show();
    m_scoreText->Show();
    CaptureKeyboard(this);
    ReDraw();
}

void MainWindow::UpdateScore(int s) {
    m_score = s;
    wchar_t buf[64]; swprintf(buf,64,L"Счёт: %d", s);
    m_scoreText->SetText(buf);
    m_scoreText->ReDraw();
}

// ======================== Board ========================

void Board::Reset() {
    m_snake.clear();
    int cx = GRID_W/2, cy = GRID_H/2;
    m_snake.push_front(Point(cx,cy));
    m_snake.push_back(Point(cx-1,cy));
    m_snake.push_back(Point(cx-2,cy));
    m_dir = Right; m_next = Right;
    m_score = 0; m_paused = false;

    SpawnFood();
    CreateTimeout(this, m_stepMs);
    ReDraw();
}

bool Board::OnKeyPress(uint64_t key) {
    switch (key) {
        case KEY_Up:    if (m_dir!=Down)  m_next=Up;    return true;
        case KEY_Down:  if (m_dir!=Up)    m_next=Down;  return true;
        case KEY_Left:  if (m_dir!=Right) m_next=Left;  return true;
        case KEY_Right: if (m_dir!=Left)  m_next=Right; return true;
        case 'p': case 'P': m_paused = !m_paused; return true;
        default: return false;
    }
}

bool Board::OnTimeout() {
    if (!m_paused) { Step(); ReDraw(); }
    return true;
}

void Board::SpawnFood() {
    std::vector<Point> free;
    free.reserve(GRID_W*GRID_H);
    for (int y=0;y<GRID_H;++y)
        for (int x=0;x<GRID_W;++x)
            if (!Occupied(x,y)) free.push_back(Point(x,y));
    if (free.empty()) { NotifyParent(EVENT_GAME_WIN, Point(0,0)); return; }
    std::uniform_int_distribution<int> d(0,(int)free.size()-1);
    m_food = free[d(m_rng)];
}

bool Board::Occupied(int x,int y) const {
    for (auto& c : m_snake) if (c.x==x && c.y==y) return true;
    return false;
}

void Board::Step() {
    if (m_snake.empty()) return;

    m_dir = m_next;
    Point head = m_snake.front();
    switch (m_dir){ case Up: --head.y; break; case Down: ++head.y; break; case Left: --head.x; break; case Right: ++head.x; break; }

    if (head.x<0 || head.x>=GRID_W || head.y<0 || head.y>=GRID_H) { NotifyParent(EVENT_GAME_OVER, Point(0,0)); return; }
    if (Occupied(head.x, head.y))                                  { NotifyParent(EVENT_GAME_OVER, Point(0,0)); return; }

    m_snake.push_front(head);
    if (head.x==m_food.x && head.y==m_food.y) {
        m_score += 10;
        NotifyParent(EVENT_SCORE_CHANGED, Point(0,0));
        SpawnFood();
    } else {
        m_snake.pop_back();
    }
}

void Board::DrawEyes(Context* cr) const {
    if (m_snake.empty()) return;
    Point h = m_snake.front();
    int x = h.x*CELL, y = h.y*CELL;
    int r = CELL/6;
    int pad = CELL/6;

    int ex1, ey1, ex2, ey2;
    switch (m_dir) {
        case Right: ex1=x+CELL-pad-r*2; ey1=y+pad;          ex2=x+CELL-pad-r*2; ey2=y+CELL-pad-r*2; break;
        case Left:  ex1=x+pad;          ey1=y+pad;          ex2=x+pad;          ey2=y+CELL-pad-r*2; break;
        case Up:    ex1=x+pad;          ey1=y+pad;          ex2=x+CELL-pad-r*2; ey2=y+pad;          break;
        case Down:  ex1=x+pad;          ey1=y+CELL-pad-r*2; ex2=x+CELL-pad-r*2; ey2=y+CELL-pad-r*2; break;
    }
    cr->SetColor(RGB(0,0,0));
    cr->FillEllipse(Point(ex1,ey1), Rect(r*2,r*2));
    cr->FillEllipse(Point(ex2,ey2), Rect(r*2,r*2));
}

void Board::DrawFood(Context* cr) const {
    // «клубника» без PNG: красный круг + маленький зелёный листик
    int x = m_food.x*CELL, y = m_food.y*CELL;
    int r = (int)(CELL*0.36);
    int cx = x + CELL/2 - r;
    int cy = y + CELL/2 - r;

    cr->SetColor(FOOD_RED);
    cr->FillEllipse(Point(cx,cy), Rect(r*2, r*2));

    // листик/хвостик
    cr->SetColor(FOOD_GREEN);
    int lw = r, lh = r/2;
    cr->FillRectangle(Point(x + CELL/2 - lw/2, y + CELL/2 - r - lh/2), Rect(lw, lh));
}

void Board::OnDraw(Context* cr) {
    // шахматная зелёная сетка
    for (int y=0;y<GRID_H;++y){
        for (int x=0;x<GRID_W;++x){
            bool light = ((x+y)&1)==0;
            cr->SetColor(light ? FIELD_LIGHT : FIELD_DARK);
            cr->FillRectangle(Point(x*CELL,y*CELL), Rect(CELL,CELL));
        }
    }

    // еда
    DrawFood(cr);

    // тело
    for (size_t i=1;i<m_snake.size();++i){
        cr->SetColor(SNAKE_BODY);
        cr->FillRectangle(Point(m_snake[i].x*CELL+1, m_snake[i].y*CELL+1), Rect(CELL-2, CELL-2));
    }
    // голова + глазки
    if (!m_snake.empty()){
        cr->SetColor(SNAKE_HEAD);
        cr->FillRectangle(Point(m_snake.front().x*CELL+1, m_snake.front().y*CELL+1), Rect(CELL-2, CELL-2));
        DrawEyes(cr);
    }
}

// ----------------------- main ------------------------
int main(int argc, char** argv){
    MainWindow* w = new MainWindow;
    int res = Run(argc, argv, w, WIN_W, WIN_H);   // 400×300
    delete w;
    return res;
}

