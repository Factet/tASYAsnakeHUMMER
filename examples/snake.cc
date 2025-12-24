#include <gtk/gtk.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstdio>

#include "../GTK/context.h"
#include "../GTK/mytypes.h"

// -------------------------------------------------
//  ПАРАМЕТРЫ ПОЛЯ И ОКНА
// -------------------------------------------------

#define GRID_W  15
#define GRID_H  15
#define CELL    40

#define FIELD_W (GRID_W * CELL)
#define FIELD_H (GRID_H * CELL)

#define WIN_W   700
#define WIN_H   800

#define TIMER_MS 150

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -------------------------------------------------
//  ЦВЕТА
// -------------------------------------------------

static const RGB COL_WINDOW_BG   = RGB(0.7, 0.7, 0.7);
static const RGB COL_FIELD_LIGHT = RGB(0.80, 0.92, 0.80);
static const RGB COL_FIELD_DARK  = RGB(0.60, 0.85, 0.60);
static const RGB COL_FIELD_FRAME = RGB(0.0, 0.0, 0.0);
static const RGB COL_SNAKE_BODY  = RGB(0.35, 0.70, 1.00);
static const RGB COL_SNAKE_HEAD  = RGB(0.10, 0.45, 0.90);
static const RGB COL_FOOD        = RGB(0.90, 0.20, 0.20);

// -------------------------------------------------
//  НАПРАВЛЕНИЯ И СОСТОЯНИЕ
// -------------------------------------------------

enum Direction
{
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

struct GameState
{
    GtkWidget *window;
    GtkWidget *drawing_area;
    GtkWidget *title_label;
    GtkWidget *score_label;
    GtkWidget *pause_label;
    GtkWidget *start_button;
    GtkWidget *restart_button;

    int       snakeX[GRID_W * GRID_H];
    int       snakeY[GRID_W * GRID_H];
    int       snakeLen;
    Direction dir;

    bool      gameOver;
    bool      paused;
    bool      inMenu;

    int       foodX;
    int       foodY;

    int       score;

    bool      shake;
    int       shakeTicks;
    int       shakeMaxTicks;
    int       shakeDX;
    int       shakeDY;

    int       fieldLeft;
    int       fieldTop;
};

// -------------------------------------------------
//  ПРОТОТИПЫ
// -------------------------------------------------

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean on_timeout(gpointer user_data);
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *, gpointer user_data);
static void     on_start(GtkButton *, gpointer user_data);
static void     on_restart(GtkButton *, gpointer user_data);

static void StartGame(GameState *state);
static void GameOver(GameState *state);
static void PlaceFood(GameState *state);
static void MoveSnake(GameState *state);
static void CheckCollision(GameState *state);
static void LayoutPause(GameState *state);
static void UpdateScore(GameState *state);

static void DrawField(GameState *state, Context *cr);
static void DrawSnake(GameState *state, Context *cr);
static void DrawFood(GameState *state, Context *cr);
static void DrawBerry(Context *cr, int cx, int cy, int size);

// -------------------------------------------------
//  MAIN
// -------------------------------------------------

int main(int argc, char **argv)
{
    std::srand(std::time(nullptr));

    gtk_init(&argc, &argv);

    GameState *state = new GameState();
    state->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(state->window), "Змейка (GTK3)");
    gtk_window_set_default_size(GTK_WINDOW(state->window), WIN_W, WIN_H);
    gtk_container_set_border_width(GTK_CONTAINER(state->window), 10);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(state->window), vbox);

    state->title_label = gtk_label_new(nullptr);
    gtk_widget_set_halign(state->title_label, GTK_ALIGN_CENTER);
    gtk_label_set_markup(GTK_LABEL(state->title_label),
                         "<span font_desc=\"32\" weight=\"bold\">ЗМЕЙКА</span>");
    gtk_box_pack_start(GTK_BOX(vbox), state->title_label, FALSE, FALSE, 0);

    state->score_label = gtk_label_new("score: 0");
    gtk_widget_set_halign(state->score_label, GTK_ALIGN_CENTER);
    gtk_label_set_markup(GTK_LABEL(state->score_label),
                         "<span font_desc=\"20\">score: 0</span>");
    gtk_box_pack_start(GTK_BOX(vbox), state->score_label, FALSE, FALSE, 0);
    gtk_widget_hide(state->score_label);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_box_pack_start(GTK_BOX(vbox), overlay, TRUE, TRUE, 0);

    state->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(state->drawing_area, TRUE);
    gtk_widget_set_vexpand(state->drawing_area, TRUE);
    gtk_widget_set_size_request(state->drawing_area, FIELD_W + 40, FIELD_H + 40);
    gtk_widget_add_events(state->drawing_area, GDK_BUTTON_PRESS_MASK);
    gtk_widget_set_can_focus(state->drawing_area, TRUE);
    gtk_container_add(GTK_CONTAINER(overlay), state->drawing_area);

    state->pause_label = gtk_label_new(nullptr);
    gtk_label_set_markup(GTK_LABEL(state->pause_label),
                         "<span font_desc=\"28\" weight=\"bold\">ПАУЗА</span>");
    gtk_widget_set_halign(state->pause_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(state->pause_label, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), state->pause_label);
    gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(overlay), state->pause_label, TRUE);
    gtk_widget_hide(state->pause_label);

    state->start_button = gtk_button_new_with_label("СТАРТ");
    gtk_widget_set_halign(state->start_button, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(state->start_button, 220, 60);
    gtk_box_pack_start(GTK_BOX(vbox), state->start_button, FALSE, FALSE, 0);

    state->restart_button = gtk_button_new_with_label("Сыграть ещё");
    gtk_widget_set_halign(state->restart_button, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(state->restart_button, 200, 40);
    gtk_box_pack_start(GTK_BOX(vbox), state->restart_button, FALSE, FALSE, 0);
    gtk_widget_hide(state->restart_button);

    g_signal_connect(state->window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);
    g_signal_connect(state->window, "key-press-event", G_CALLBACK(on_key_press), state);
    g_signal_connect(state->drawing_area, "draw", G_CALLBACK(on_draw), state);
    g_signal_connect(state->drawing_area, "button-press-event", G_CALLBACK(on_button_press), state);
    g_signal_connect(state->start_button, "clicked", G_CALLBACK(on_start), state);
    g_signal_connect(state->restart_button, "clicked", G_CALLBACK(on_restart), state);

    g_timeout_add(TIMER_MS, on_timeout, state);

    state->inMenu    = true;
    state->gameOver  = false;
    state->paused    = false;
    state->score     = 0;
    state->shake     = false;
    state->shakeTicks    = 0;
    state->shakeMaxTicks = 0;
    state->shakeDX       = 0;
    state->shakeDY       = 0;
    state->fieldLeft     = 0;
    state->fieldTop      = 0;

    gtk_widget_show_all(state->window);
    gtk_widget_hide(state->restart_button);
    gtk_widget_hide(state->score_label);
    gtk_widget_hide(state->pause_label);

    gtk_main();

    delete state;
    return 0;
}

// -------------------------------------------------
//  СОБЫТИЯ
// -------------------------------------------------

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    (void)widget;
    GameState *state = static_cast<GameState *>(user_data);
    guint key = event->keyval;

    if (key == 'q' || key == 'Q')
    {
        gtk_main_quit();
        return TRUE;
    }

    if (state->inMenu)
    {
        if (key == GDK_KEY_Return || key == ' ')
        {
            StartGame(state);
        }
        return TRUE;
    }

    if ((key == 'p' || key == 'P') && !state->gameOver)
    {
        state->paused = !state->paused;
        LayoutPause(state);
        gtk_widget_queue_draw(state->drawing_area);
        return TRUE;
    }

    if (!state->gameOver && !state->paused)
    {
        switch (key)
        {
            case GDK_KEY_Up:
                if (state->dir != DIR_DOWN) state->dir = DIR_UP;
                break;
            case GDK_KEY_Down:
                if (state->dir != DIR_UP) state->dir = DIR_DOWN;
                break;
            case GDK_KEY_Left:
                if (state->dir != DIR_RIGHT) state->dir = DIR_LEFT;
                break;
            case GDK_KEY_Right:
                if (state->dir != DIR_LEFT) state->dir = DIR_RIGHT;
                break;
            default:
                ;
        }
    }
    return TRUE;
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *, gpointer user_data)
{
    GameState *state = static_cast<GameState *>(user_data);
    gtk_widget_grab_focus(widget);
    return TRUE;
}

static gboolean on_timeout(gpointer user_data)
{
    GameState *state = static_cast<GameState *>(user_data);

    if (!state->inMenu && !state->gameOver && !state->paused)
    {
        MoveSnake(state);
        CheckCollision(state);
        gtk_widget_queue_draw(state->drawing_area);
    }
    else if (!state->inMenu && state->gameOver && state->shake)
    {
        state->shakeTicks++;
        state->shakeDX = (std::rand() % 7) - 3;
        state->shakeDY = 0;

        if (state->shakeTicks >= state->shakeMaxTicks)
        {
            state->shake = false;
            state->shakeDX = 0;
            state->shakeDY = 0;
        }
        gtk_widget_queue_draw(state->drawing_area);
    }
    return TRUE;
}

static void on_start(GtkButton *, gpointer user_data)
{
    StartGame(static_cast<GameState *>(user_data));
}

static void on_restart(GtkButton *, gpointer user_data)
{
    StartGame(static_cast<GameState *>(user_data));
}

// -------------------------------------------------
//  ЛОГИКА
// -------------------------------------------------

static void StartGame(GameState *state)
{
    state->inMenu   = false;
    state->gameOver = false;
    state->paused   = false;
    state->score    = 0;

    gtk_label_set_markup(GTK_LABEL(state->title_label),
                         "<span font_desc=\"32\" weight=\"bold\">ЗМЕЙКА</span>");
    gtk_widget_show(state->score_label);
    gtk_widget_hide(state->restart_button);
    gtk_widget_hide(state->pause_label);
    gtk_widget_hide(state->start_button);
    LayoutPause(state);
    UpdateScore(state);

    state->shake         = false;
    state->shakeTicks    = 0;
    state->shakeMaxTicks = 0;
    state->shakeDX       = 0;
    state->shakeDY       = 0;

    int cx = GRID_W / 2;
    int cy = GRID_H / 2;
    state->snakeLen = 3;
    state->snakeX[0] = cx;
    state->snakeY[0] = cy;
    state->snakeX[1] = cx - 1;
    state->snakeY[1] = cy;
    state->snakeX[2] = cx - 2;
    state->snakeY[2] = cy;

    state->dir = DIR_RIGHT;

    PlaceFood(state);

    gtk_widget_grab_focus(state->drawing_area);
    gtk_widget_queue_draw(state->drawing_area);
}

static void GameOver(GameState *state)
{
    if (state->gameOver) return;

    state->gameOver = true;
    gtk_label_set_markup(GTK_LABEL(state->title_label),
                         "<span font_desc=\"32\" weight=\"bold\">ПРОИГРЫШ</span>");
    gtk_widget_show(state->restart_button);
    gtk_widget_hide(state->pause_label);

    state->shake         = true;
    state->shakeTicks    = 0;
    state->shakeMaxTicks = 25;
    state->shakeDX       = 0;
    state->shakeDY       = 0;

    gtk_widget_queue_draw(state->drawing_area);
}

static void PlaceFood(GameState *state)
{
    while (true)
    {
        state->foodX = std::rand() % GRID_W;
        state->foodY = std::rand() % GRID_H;

        bool clash = false;
        for (int i = 0; i < state->snakeLen; ++i)
        {
            if (state->snakeX[i] == state->foodX && state->snakeY[i] == state->foodY)
            {
                clash = true;
                break;
            }
        }
        if (!clash) break;
    }
}

static void MoveSnake(GameState *state)
{
    int oldTailX = state->snakeX[state->snakeLen - 1];
    int oldTailY = state->snakeY[state->snakeLen - 1];

    for (int i = state->snakeLen - 1; i > 0; --i)
    {
        state->snakeX[i] = state->snakeX[i - 1];
        state->snakeY[i] = state->snakeY[i - 1];
    }

    switch (state->dir)
    {
        case DIR_UP:    state->snakeY[0]--; break;
        case DIR_DOWN:  state->snakeY[0]++; break;
        case DIR_LEFT:  state->snakeX[0]--; break;
        case DIR_RIGHT: state->snakeX[0]++; break;
    }

    if (state->snakeX[0] == state->foodX && state->snakeY[0] == state->foodY)
    {
        if (state->snakeLen < GRID_W * GRID_H)
        {
            state->snakeX[state->snakeLen] = oldTailX;
            state->snakeY[state->snakeLen] = oldTailY;
            state->snakeLen++;
        }
        state->score += 10;
        UpdateScore(state);
        PlaceFood(state);
    }
}

static void CheckCollision(GameState *state)
{
    if (state->snakeX[0] < 0 || state->snakeX[0] >= GRID_W ||
        state->snakeY[0] < 0 || state->snakeY[0] >= GRID_H)
    {
        GameOver(state);
        return;
    }

    for (int i = 1; i < state->snakeLen; ++i)
    {
        if (state->snakeX[i] == state->snakeX[0] &&
            state->snakeY[i] == state->snakeY[0])
        {
            GameOver(state);
            return;
        }
    }
}

static void UpdateScore(GameState *state)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "<span font_desc=\"20\">score: %d</span>", state->score);
    gtk_label_set_markup(GTK_LABEL(state->score_label), buf);
}

static void LayoutPause(GameState *state)
{
    if (state->paused) gtk_widget_show(state->pause_label);
    else               gtk_widget_hide(state->pause_label);
}

// -------------------------------------------------
//  ОТРИСОВКА
// -------------------------------------------------

static void ComputeFieldPosition(GameState *state, GtkWidget *widget)
{
    GtkAllocation alc;
    gtk_widget_get_allocation(widget, &alc);

    int winW = alc.width;
    int winH = alc.height;

    int centeredTop = (winH - FIELD_H) / 2;
    if (centeredTop < 10) centeredTop = 10;
    state->fieldTop = centeredTop;

    state->fieldLeft = (winW - FIELD_W) / 2;
    if (state->fieldLeft < 10) state->fieldLeft = 10;
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    GameState *state = static_cast<GameState *>(user_data);
    CairoContext ctx(cr);

    GtkAllocation alc;
    gtk_widget_get_allocation(widget, &alc);
    Rect r(alc.width, alc.height);

    ctx.SetColor(COL_WINDOW_BG);
    ctx.FillRectangle(Point(0, 0), r);

    if (state->inMenu)
    {
        return TRUE;
    }

    ComputeFieldPosition(state, widget);

    DrawField(state, &ctx);
    DrawSnake(state, &ctx);
    DrawFood(state, &ctx);

    return TRUE;
}

static void DrawField(GameState *state, Context *cr)
{
    int baseLeft = state->fieldLeft + state->shakeDX;
    int baseTop  = state->fieldTop  + state->shakeDY;

    cr->SetColor(COL_FIELD_FRAME);
    cr->FillRectangle(Point(baseLeft - 2, baseTop - 2), Rect(FIELD_W + 4, 2));
    cr->FillRectangle(Point(baseLeft - 2, baseTop + FIELD_H), Rect(FIELD_W + 4, 2));
    cr->FillRectangle(Point(baseLeft - 2, baseTop - 2), Rect(2, FIELD_H + 4));
    cr->FillRectangle(Point(baseLeft + FIELD_W, baseTop - 2), Rect(2, FIELD_H + 4));

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

static void DrawSnake(GameState *state, Context *cr)
{
    int baseLeft = state->fieldLeft + state->shakeDX;
    int baseTop  = state->fieldTop  + state->shakeDY;

    for (int i = 1; i < state->snakeLen; ++i)
    {
        int px = baseLeft + state->snakeX[i] * CELL;
        int py = baseTop  + state->snakeY[i] * CELL;

        cr->SetColor(COL_SNAKE_BODY);
        cr->FillRectangle(Point(px + 1, py + 1), Rect(CELL - 2, CELL - 2));
    }

    int hx = baseLeft + state->snakeX[0] * CELL;
    int hy = baseTop  + state->snakeY[0] * CELL;

    cr->SetColor(COL_SNAKE_HEAD);
    cr->FillRectangle(Point(hx + 1, hy + 1), Rect(CELL - 2, CELL - 2));

    int eyeSize = 4;
    int margin  = 5;
    int midX    = hx + CELL / 2;
    int midY    = hy + CELL / 2;

    cr->SetColor(RGB(0.0, 0.0, 0.0));

    switch (state->dir)
    {
        case DIR_RIGHT:
        {
            int ex  = hx + CELL - margin - eyeSize;
            int ey1 = midY - eyeSize - 1;
            int ey2 = midY + 1;
            cr->FillRectangle(Point(ex, ey1), Rect(eyeSize, eyeSize));
            cr->FillRectangle(Point(ex, ey2), Rect(eyeSize, eyeSize));
            break;
        }
        case DIR_LEFT:
        {
            int ex  = hx + margin;
            int ey1 = midY - eyeSize - 1;
            int ey2 = midY + 1;
            cr->FillRectangle(Point(ex, ey1), Rect(eyeSize, eyeSize));
            cr->FillRectangle(Point(ex, ey2), Rect(eyeSize, eyeSize));
            break;
        }
        case DIR_UP:
        {
            int ey  = hy + margin;
            int ex1 = midX - eyeSize - 1;
            int ex2 = midX + 1;
            cr->FillRectangle(Point(ex1, ey), Rect(eyeSize, eyeSize));
            cr->FillRectangle(Point(ex2, ey), Rect(eyeSize, eyeSize));
            break;
        }
        case DIR_DOWN:
        {
            int ey  = hy + CELL - margin - eyeSize;
            int ex1 = midX - eyeSize - 1;
            int ex2 = midX + 1;
            cr->FillRectangle(Point(ex1, ey), Rect(eyeSize, eyeSize));
            cr->FillRectangle(Point(ex2, ey), Rect(eyeSize, eyeSize));
            break;
        }
    }
}

static void DrawBerry(Context *cr, int cx, int cy, int size)
{
    int rx = size / 2 - 4;
    int ry = size / 2;

    const int N = 32;
    Point pts[N];

    for (int i = 0; i < N; ++i)
    {
        double angle = 2.0 * M_PI * i / N;
        double ca = std::cos(angle);
        double sa = std::sin(angle);

        double x, y;

        if (sa < 0)
        {
            x = cx + rx * ca * 0.8;
            y = cy + ry * sa * 0.35;
        }
        else
        {
            x = cx + rx * ca * 1.05;
            y = cy + ry * sa * 1.15;
        }

        pts[i] = Point((int)x, (int)y);
    }

    cr->SetColor(COL_FOOD);
    cr->FillPolyline(N, pts);

    int topY = cy - ry * 0.5;
    Point leaf[3];
    leaf[0] = Point(cx - 7, topY + 3);
    leaf[1] = Point(cx + 7, topY + 3);
    leaf[2] = Point(cx,     topY - 6);
    cr->SetColor(RGB(0.0, 0.6, 0.0));
    cr->FillPolyline(3, leaf);

    cr->FillRectangle(Point(cx - 1, topY - 8), Rect(2, 5));

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

static void DrawFood(GameState *state, Context *cr)
{
    int baseLeft = state->fieldLeft + state->shakeDX;
    int baseTop  = state->fieldTop  + state->shakeDY;

    int cellX = baseLeft + state->foodX * CELL;
    int cellY = baseTop  + state->foodY * CELL;

    int cx = cellX + CELL / 2;
    int cy = cellY + CELL / 2 + 2;

    DrawBerry(cr, cx, cy, CELL);
}
