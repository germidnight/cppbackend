Задание 2 — Морской бой по TCP
Правила игры: каждый игрок на поле 8х8 располагает 10 кораблей:
- 1 четырёхпалубный (занимающий 4 клетки),
- 2 трёхпалубных,
- 3 двухпалубных,
- 4 однопалубных.
Корабли не могут накладываться друг на друга, а также соприкасаться по горизонтали, диагонали и вертикали.
Расстановка кораблей производится случайно, она определяется сидом, который передаётся при запуске игры.

После начала игры игроки делают выстрелы, не видя расположение кораблей соперника. При промахе ход передаётся другому игроку.
При попадании даётся право ещё одного выстрела. Соперник сообщает результат выстрела:
- «мимо»,
- «попал»,
- «убил».
Последний статус означает, что поражены все клетки, занимаемые кораблём.

Механика игрового поля реализована в классе SeabattleField, задающем игровое поле. Этот класс реализован полностью, менять его не нужно.
Вам понадобятся следующие методы:
1) Статический метод GetRandomField, принимающий генератор случайных чисел. Используется для инициализации поля игрока.
2) Конструктор. Используется для инициализации поля соперника.
3) Shoot(size_t x, size_t y) — произвести выстрел по сектору: получить результат выстрела.
Используется для своего поля при выстреле соперника.
4) MarkMiss(size_t x, size_t y) - отметить промах (мимо),
5) MarkHit(size_t x, size_t y) - отметить попадание (попал),
6) MarkKill(size_t x, size_t y) - отметить результат потопления корабля (убил).
Используется для поля соперника.
7) IsLoser — возвращает true, если все корабли на поле уничтожены.

Механика игры частично реализована в классе SeabattleAgent. Вам пригодятся следующие методы и поля класса:
1) Конструктор — принимает поле игрока с уже расставленными кораблями.
2) StartGame (заготовка) — этот метод должен содержать всю логику игры. Вам предстоит его реализовать.
3) ParseMove — преобразует текстовое представление клетки, состоящее из буквы и цифры (например, B6) в координаты.
Возвращает пару чисел, в которой первый элемент соответствует букве (значение от 0 до 7), а второй - цифре (также от 0 до 7).
В случае некорректной клетки возвращает std::nullopt.
4) MoveToString — преобразует координаты в текстовое представление клетки.
5) PrintFields — выводит в cout два поля: игрока и соперника.
6) IsGameEnded — возвращает true, если игра завершена.
 - my_field_ — поле игрока.
 - other_field_ — поле соперника.

Каждая клетка поля может находиться в одном из следующих статусов:
- UNKNOWN — содержимое неизвестно. Применяется для поля соперника по умолчанию.
- EMPTY — клетка пуста. Применяется как для поля игрока, так и для открытых клеток соперника.
- KILLED — поражённая клетка с кораблём. Применяется как для поля игрока, так и для открытых клеток соперника.
- SHIP — клетка с непоражённым кораблём. Применяется для поля игрока.
Клиент и сервер будут играть по сети. Каждый из них независимо выбирает сид. Сид — это значение, которое позволяет
инициализировать счётчик псевдослучайных чисел и определяет все последующие значения.
Если запустить приложение, использующее в своей работе псевдослучайные числа, с известным сидом, результат работы можно предсказать.
Сид определяет расположение кораблей и задаётся параметром командной строки.

Начинает клиент.

Протокол взаимодействия:
1) При выстреле игрок передаёт другому игроку два байта, содержащие текстовое представление координаты, например, C7.
2) Соперник сообщает ответ, содержащий один байт — результат выстрела. Он кодируется преобразованием значения типа SeabattleField::ShotResult к char.

Параметры командной строки:
Клиент и сервер — одна программа. Различие будет в параметрах командной строки.
- Сервер принимает два параметра — сид и порт.
- Клиент принимает три параметра — сид, IP-сервера, порт.