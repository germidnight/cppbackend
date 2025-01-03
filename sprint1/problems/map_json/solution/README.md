Разработайте веб-сервер, предоставляющий клиентам REST API со списком карт.

Формат входных данных
1) Программе через единственный параметр командной строки подаётся путь к конфигурационному файлу в формате JSON, содержащему информацию об игровом поле.
Следующая команда запускает сервер и передаёт ему файл config.json:
game_server config.json

2) Файл содержит JSON-объект с единственным свойством maps. Это свойство описывает массив игровых карт или уровней.
{
    "maps": []
}
Каждый элемент этого массива — объект, описывающий дороги, здания и офисы на игровой карте. Свойства этого объекта:
- id — уникальный идентификатор карты. Тип: строка.
- name — название карты, которое выводится пользователю. Тип: строка.
- roads — дороги игровой карты. Тип: массив объектов. Массив должен содержать хотя бы один элемент.
- buildings — здания. Тип: массив объектов. Массив может быть пустым.
- offices — офисы бюро находок. Тип: массив объектов. Массив может быть пустым.

В массиве roads содержатся элементы двух типов, каждый из которых описывает горизонтальный либо вертикальный отрезок дороги. Порядок следования элементов в этом массиве может быть произвольным.
- Горизонтальный отрезок дороги — объект со свойствами x0, y0, x1, задающих целочисленные значения координат начала и конца отрезка. Пример:
{ "x0": 0, "y0": 0, "x1": 40 }
- Вертикальный описывается объектом со свойствами x0, y0, y1, которые задают целочисленные значения координат начала и конца отрезка дороги. Пример:
{ "x0": 0, "y0": 0, "y1": 20 }

Здание на карте представлено в виде объекта со свойствами x, y, w, h, которые задают целочисленные координаты верхнего левого угла дома, его ширину и высоту.
Пример:
{ "x": 5, "y": 5, "w": 30, "h": 20 }

Офис представлен объектом со свойствами:
id — уникальный идентификатор офиса. Строка.
x, y — координаты офиса. Целые числа. Офисы располагаются на дороге, так как собаки могут перемещаться только по дорогам.
offsetX, offsetY — смещение изображения офиса относительно координат x и y. Эти параметры используются только на стороне клиента для визуализации офиса и на игровую логику не влияют.
Пример описания офиса:
{
  "id": "o0",
   "x": 40,
   "y": 30,
   "offsetX": 5,
   "offsetY": 0
}
Пример конфигурационного файла:
{
  "maps": [
    {
      "id": "map1",
      "name": "Map 1",
      "roads": [
        { "x0": 0, "y0": 0, "x1": 40 },
        { "x0": 40, "y0": 0, "y1": 30 },
        { "x0": 40, "y0": 30, "x1": 0 },
        { "x0": 0, "y0": 0, "y1": 30 }
      ],
      "buildings": [
        { "x": 5, "y": 5, "w": 30, "h": 20 }
      ],
      "offices": [
        { "id": "o0", "x": 40, "y": 30, "offsetX": 5, "offsetY": 0 }
      ]
    }
  ]
}
В конфигурационном файле содержится одна карта с названием “Map 1” и id, равным “map1”. Карта состоит из 4 отрезков дороги, 1 здания и 1 бюро находок.

Описание поведения веб-сервера
Сервер должен принимать HTTP-запросы на порт 8080. Нужно реализовать поддержку запросов двух типов:
1) получение списка карт,
/api/v1/maps — GET-запрос возвращает в теле ответа краткую информацию обо всех картах в виде JSON-массива объектов с полями id и name. Ответ должен иметь обязательный заголовок Content-Type со значением application/json. Карты в ответе должны быть перечислены в том же порядке, что и в конфигурационном файле.
Пример тела ответа:
[{"id": "map1", "name": "Map 1"}]

2) получение описания карты с заданным id.
/api/v1/maps/{id-карты} — GET-запрос возвращает в теле ответа JSON-описание карты с указанным id, семантически эквивалентное представлению карты из конфигурационного файла. Ответ сервера должен иметь заголовок Content-Type со значением application/json.
Пример тела ответа на запрос GET /api/v1/maps/map1 HTTP/1.1:
{
  "id": "map1",
  "name": "Map 1",
  "roads": [
    { "x0": 0, "y0": 0, "x1": 40 },
    { "x0": 40, "y0": 0, "y1": 30 },
    { "x0": 40, "y0": 30, "x1": 0 },
    { "x0": 0, "y0": 0, "y1": 30 }
  ],
  "buildings": [
    { "x": 5, "y": 5, "w": 30, "h": 20 }
  ],
  "offices": [
    { "id": "o0", "x": 40, "y": 30, "offsetX": 5, "offsetY": 0 }
  ]
}

Обработка ошибок
В случае ошибки сервер должен возвращать ответ с соответствующим HTTP-кодом ошибки. Заголовок Content-Type ответа сервера должен иметь значение application/json. Тело ответа — JSON-объект с полями code и message, содержащими код ошибки и её понятное описание.
1) Если в запросе на получение описания карты указан id несуществующей карты, сервер должен вернуть ответ со статус-кодом 404 и следующим телом ответа:
{
  "code": "mapNotFound",
  "message": "Map not found"
}
Поле message может содержать и другой понятный человеку текст, однако поле code должно иметь значение “mapNotFound”.
2) Если URI-строка запроса начинается с /api/, но не подпадает ни под один из текущих форматов, сервер должен вернуть ответ с 400 статус-кодом. Тело ответа должно содержать такой JSON-объект:
{
  "code": "badRequest",
  "message": "Bad request"
}
Здесь поле message также может иметь другой понятный текст, однако поле code должно иметь значение “badRequest”.

Обработка сигналов
После своего старта сервер должен реагировать на сигналы SIGINT и SIGTERM и корректно завершать свою работу при получении этих сигналов.