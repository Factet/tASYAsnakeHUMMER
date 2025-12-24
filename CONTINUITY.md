Goal (incl. success criteria):
- Переписать игру змейка (пример myprog) на GTK3 с сохранением поведения; исправить сборочные ошибки (типы Point/Rect/RGB, wchar_t строки), доработать глаза и семечки клубники.

Constraints/Assumptions:
- Ответы на русском.
- Следовать инструкциям: коммит + PR после изменений.
- AGENTS.md не обнаружено.
- Сохранить все особенности интерфейса/управления/поведения; допускаются небольшие изменения интерфейса.
- Использовать pkg-config gtk+-3.0 и обновить makefile для сборки с GTK.

Key decisions:
- Версия змейки на GTK3 (examples/snake.cc) с CairoContext/Context; внесены правки в глаза и семечки клубники.
- Добавлены необходимые include для типов (GTK/context.h подключает mytypes.h; GTK/mytypes.h подключает <ostream>); предстоит перейти на узкие строки для text.h.

State:
- Графические правки внесены и закоммичены; исправлены include для типов; предстоят правки строк/Point в snake.cc; сборка GTK3 по-прежнему недоступна.

Done:
- Порт на GTK3, makefile обновлён, визуальные правки сделаны; добавлены include для устранения части ошибок компиляции.

Now:
- Починить ошибки сборки (wchar_t строки → UTF-8, доступ к полям Point, убрать FillEllipse и т.п.); сборку проверить нельзя из-за отсутствия gtk+-3.0/pkg-config.

Next:
- Оформить коммит и обновить PR после правок.

Open questions:
- Нет.

Working set (files/ids/commands):
- CONTINUITY.md; GTK/context.h; GTK/mytypes.h; examples/snake.cc; makefile; ошибки сборки gtk+-3.0 (pkg-config/apt 403).
