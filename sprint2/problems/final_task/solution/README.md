Разработайте веб-сервер игры
/* ------------------------------------------------------------------------------------ */
**1) Предоставляю клиентам REST API со списком карт.

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

/* ------------------------------------------------------------------------------------ */
**2) Добавил возможность отдавать содержимое статических файлов, расположенных внутри заданного каталога.

Входные данные:
Имя каталога файлами клиентской части игры передаётся в аргументах командной строки при запуске

Выходные данные
GET- и HEAD-запросы, в которых URI-строка начинается не с /api/, должны интерпретироваться сервером как запросы статических файлов внутри соответствующего каталога.
Например, в теле ответа на запрос GET /images/image1.png HTTP/1.1 сервер должен отдать содержимое файла images/image1.png относительно каталога со статическими файлами. Ответ должен содержать следующие заголовки ответа:
- Content-Type — MIME-тип контента, зависящий от расширения файла.
- Content-Length — размер файла в байтах
Значение заголовка Content-Type должно зависеть от расширения запрашиваемого файла:
- .htm, .html: text/html
- .css: text/css
- .txt: text/plain
- .js: text/javascript
- .json: application/json
- .xml: application/xml
- .png: image/png
- .jpg, .jpe, .jpeg: image/jpeg
- .gif: image/gif
- .bmp: image/bmp
- .ico: image/vnd.microsoft.icon
- .tiff, .tif: image/tiff
- .svg, .svgz: image/svg+xml
- .mp3: audio/mpeg
- Для файлов с пустым или неизвестным расширением заголовок Content-Type должен иметь значение application/octet-stream.
Определение MIME-типа по расширению файла должно выполняться без учёта регистра символов.

Если URI-строка запроса задаёт путь к каталогу в файловой системе, должно вернуться содержимое файла index.html из этого каталога. Например, следующие строки запроса ссылаются на файл index.html в корневой директории веб-сервера.
GET / HTTP/1.1
GET /index.html HTTP/1.1
Сервер должен трактовать URI-строку запроса как URL-encoded. Например, по запросу GET /hello%20world.txt HTTP/1.1 должно вернуться содержимое файла hello world.txt из корневого каталога веб-сервера. Необходимо осуществлять URL-декодирование строки.

Обработка ошибок
- Если URI-строка ссылается на несуществующий файл, сервер должен вернуть ответ со статус-кодом 404 Not Found и Content-Type: text/plain. Текст в ответе остаётся на ваше усмотрение.
- Если результирующий путь к файлу оказался вне корневого каталога со статическими файлами, сервер должен вернуть ответ со статус-кодом 400 Bad Request и Content-Type: text/plain. Текст ошибки в ответе остаётся на ваше усмотрение.

/* ------------------------------------------------------------------------------------ */
**3) Реализовать логирование

Для логирования в нашем игровом сервере будем использвать Boost.Log.
Логи будем писать в стандартный вывод, чтобы их удобно было читать извне docker-контейнера.
Выберем для этого формат JSON: будем записывать отдельный объект на каждой строке.
JSON-запись представляет собой объект с такими полями:
- timestamp — время логирования в формате ISO (полученное из объекта записи и преобразованное функцией to_iso_extended_string),
- message — сообщение,
- data — произвольные данные в виде JSON-объекта, переданные в лог через атрибут.

В сервере нужно логировать следующие события:
1) запуск сервера,
2) остановку сервера,
3) получение запроса,
4) формирование ответа,
5) возникновение ошибки.

Чтобы при логировании выводить JSON-сообщения, нужно задать собственный форматер.
В нём представьте окончательный объект в виде boost::json::value, а затем выведите его в поток логгера.

Формат выходных данных
Ниже приводятся значения полей записи, которые должны поступать в stdout при каждом случае логирования:
1) При запуске сервера:
  а) message — строка "server started";
  б) data — объект с полями:
    - port — порт, на котором работает сервер (обычно 8080),
    - address — адрес интерфейса (обычно 0.0.0.0).
2) При остановке сервера:
  а) message — строка "server exited";
  б) data — объект с полями:
    - code — код возврата (0 при успехе, EXIT_FAILURE при ошибке),
    - exception — если выход по исключению, то описание исключения (std::exception::what()).
3) При получении запроса:
  а) message — строка "request received";
  б) data — объект с полями:
    - ip — IP-адрес клиента (полученный через endpoint.address().to_string()),
    - URI — запрошенный адрес,
    - method — использованный метод HTTP.
4) При формировании ответа:
  а) message — строка "response sent";
  б) data — объект с полями:
    - ip — IP-адрес клиента (полученный через endpoint.address().to_string()),
    - response_time — время формирования ответа в миллисекундах (целое число).
    - code — статус-код ответа, например, 200 (http::response<T>::result_int()).
    - content_type — строка или null, если заголовок в ответе отсутствует.
5) При возникновении сетевой ошибки:
  а) message — строка "error";
  б) data — объект с полями:
    - code — код ошибки (beast::error_code::value()).
    - text — сообщение ошибки (beast::error_code::message()).
    - where — место возникновения (read, write, accept).
Также ко всем выводимым сообщениям добавляется поле "timestamp".

/* ------------------------------------------------------------------------------------ */
**4) Реализовать вход на сервер и получение списка карт сессии игрока.

Реализуйте в игровом сервере операции входа в игру и получения информации об игроках.
Пользователь может присоединиться к игре на любой из доступных карт.
Для входа в игру он должен сообщить серверу кличку своего пса и id карты.

Сервер должен создать пса на указанной карте и игрока, управляющего псом, а затем вернуть клиенту
id игрока и токен для аутентификации. Id нужен клиенту, чтобы отличать своего игрока от других.
Клиент должен предъявить токен серверу, чтобы получать информацию о состоянии игрового сеанса и
управлять своим игроком.

- Вход в игру
Для входа в игру реализуйте обработку POST-запроса к точке входа /api/v1/game/join.
Параметры запроса:
а) Обязательный заголовок Content-Type должен иметь тип application/json.
б) Тело запроса — JSON-объект с обязательными полями userName и mapId: имя игрока и id карты. Имя игрока совпадает с именем пса.

В случае успеха должен возвращаться ответ, обладающий следующими свойствами:
а) Статус-код 200 OK.
б) Заголовок Content-Type должен иметь тип application/json.
в) Заголовок Content-Length должен хранить размер тела ответа.
г) Обязательный заголовок Cache-Control должен иметь значение no-cache.
д) Тело ответа — JSON-объект с полями authToken и playerId:
     - Поле playerId — целое число, задающее id игрока.
     - Поле authToken — токен для авторизации в игре — строка, состоящая из 32 случайных шестнадцатеричных цифр.

Если в качестве mapId указан несуществующий id карты, должен вернуться ответ:
а) со статус-кодом 404 Not found
б) заголовками:
    1) Content-Length: <размер тела ответа>,
    2) Content-Type: application/json,
    3) Cache-Control: no-cache.
в) Тело ответа — JSON-объект с полями code и message:
    1) Поле code — строка "mapNotFound".
    2) Поле message — строка с понятным человеку текстом ошибки.

Если было передано пустое имя игрока, должен вернуться ответ:
а) со статус-кодом 400 Bad request
б) заголовками:
    1) Content-Length: <размер тела ответа>,
    2) Content-Type: application/json,
    3) Cache-Control: no-cache.
в) Тело ответа — JSON-объект с полями code и message:
    1) Поле code — строка "invalidArgument".
    2) Поле message — строка с понятным человеку текстом ошибки.

Если при парсинге JSON или получении его свойств произошла ошибка, должен вернуться ответ:
а) со статус-кодом 400 Bad request
б) заголовками:
    1) Content-Length: <размер тела ответа>,
    2) Content-Type: application/json
    3) Cache-Control: no-cache.
в) Тело ответа — JSON-объект с полями code и message:
    1) Поле code — строка "invalidArgument".
    2) Поле message — строка с понятным человеку текстом ошибки.

Если метод запроса отличается от POST, должен вернуться ответ:
а) со статус-кодом 405 Method Not Allowed
б) заголовками:
    1) Content-Length: <размер тела ответа>,
    2) Content-Type: application/json,
    3) Allow: POST,
    4) Cache-Control: no-cache.
в) Тело ответа — JSON-объект с полями code и message:
    1) Поле code — строка "invalidMethod".
    2) Поле message — строка с понятным человеку текстом ошибки.

- Получение списка игроков
Чтобы получить список игроков, находящихся в одной игровой сессии с игроком, используется GET-запрос к точке входа /api/v1/game/players. Параметры запроса:
а) Обязательный заголовок Authorization: Bearer <токен пользователя>.
В качестве токена пользователя следует передать токен, полученный при входе в игру.
Этот токен сервер использует, чтобы аутентифицировать игрока и определить, на какой карте он находится.

В случае успеха должен вернуться ответ со следующими свойствами:
а) Статус-код 200 OK.
б) Заголовок Content-Type: application/json.
в) Заголовок Content-Length: <размер тела ответа>.
г) Заголовок Cache-Control: no-cache.
д) Тело ответа — JSON-объект. Его ключи — идентификаторы пользователей на карте.
Значение каждого из этих ключей — JSON-объект с единственным полем name, строкой, задающей имя пользователя, под которым он вошёл в игру.

Если заголовок Authorization в запросе отсутствует либо его значение не удовлетворяет требованиям задачи, должен вернуться ответ со следующими свойствами:
а) Статус-код 401 Unauthorized.
б) Заголовок Content-Type: appication/json.
в) Заголовок Content-Length: <размер тела ответа>.
г) Заголовок Cache-Control: no-cache.
д) Тело ответа — JSON-объект с полями code и message:
    1) Поле code должно иметь значение "invalidToken".
    2) Поле message — содержать понятное человеку описание ошибки.

Если заголовок Authorization содержит валидное значение токена, но в игре нет пользователя с таким токеном, должен вернуться ответ со следующими свойствами:
а) Статус-код 401 Unauthorized.
б) Заголовок Content-Type: appication/json.
в) Заголовок Content-Length: <размер тела ответа>.
г) Заголовок Cache-Control: no-cache.
д) Тело ответа — JSON-объект с полями code и message:
    1) Поле code должно иметь значение "unknownToken".
    2) Поле message — содержать понятное человеку описание ошибки.

Если метод запроса отличается от GET или HEAD, должен вернуться ответ со следующими свойствами:
а) Статус-код 405 Method Not Allowed.
б) Заголовок Content-Type: appication/json.
в) Заголовок Allow: GET, HEAD.
г) Заголовок Content-Length: <размер тела ответа>.
д) Заголовок Cache-Control: no-cache.
е) Тело ответа — JSON-объект с полями code и message:
    1) Поле code должно иметь значение "invalidMethod".
    2) Поле message — содержать понятное человеку описание ошибки.

/* ------------------------------------------------------------------------------------ */
**5) Реализовать в игровом сервере операции получения игрового состояния и управления действиями персонажей.

- Добавьте в модель пса информацию о его координатах, скорости и направлении в пространстве:
  1) Координаты пса на карте задаются двумя вещественными числами. Для описания вещественных координат разработайте структуру или класс.
  2) Скорость пса на карте задаётся также двумя вещественными числами. Скорость измеряется в единицах карты за одну секунду.
  3) Направление в пространстве принимает одно из четырех значений: NORTH (север), SOUTH (юг), WEST (запад), EAST (восток).
После добавления на карту пёс должен иметь имеет скорость, равную нулю.
Координаты пса — случайно выбранная точка на случайно выбранном отрезке дороги этой карты.
Направление пса по умолчанию — на север.

- Чтобы получить информацию о состоянии игры, реализуйте обработку GET-запроса к точке входа /api/v1/game/state.
Параметры запроса:
а) Обязательный заголовок Authorization: Bearer <токен пользователя> передаёт токен пользователя, полученный при входе в игру.

В случае успеха сервер должен вернуть ответ, обладающий свойствами:
а) Статус-код 200 OK.
б) Заголовок Content-Type: application/json.
в) Заголовок Content-Length: <размер тела ответа>.
г) Обязательный заголовок Cache-Control: no-cache.
д) Тело ответа — JSON объект, в теле ответа содержит свойство players — объект, ключи которого — это id игроков,
а значения — это объекты с полями: pos, speed и dir:
  1) pos — массив из двух вещественных чисел, координат x и y игрока на карте.
  2) speed — массив из двух вещественных чисел: скорость пса, которым управляет игрок по горизонтальной и вертикальной оси.
  Измеряется скорость в координатных единицах за секунду. Управление персонажами будет реализовано позже, поэтому скорость объектов пока равна нулю.
  3) dir — строка, задающая направление главного героя:
    - "L"  — влево (на запад),
    - "R" — вправо (на восток),
    - "U" — вверх (на север),
    - "D" — вниз (на юг).
  Управление персонажами будет реализовано позже, поэтому должно возвращаться направление по умолчанию "U".

Если метод запроса отличается от GET или HEAD, должен вернуться ответ со следующими свойствами:
а) Статус-код 405 Method Not Allowed.
б) Заголовок Allow: GET, HEAD.
в) Заголовок Content-Type: application/json.
г) Заголовок Content-Length: <размер тела ответа>.
д) Заголовок Cache-Control: no-cache.
е) Тело ответа — JSON-объект с полями code и message:
    1) Поле code должно иметь значение "invalidMethod".
    2) Поле message — содержать понятное человеку описание ошибки.

Если заголовок Authorization в запросе отсутствует либо его значение не соответствует формату Bearer <токен>, должен вернуться ответ со следующими свойствами:
а) Статус-код 401 Unauthorized.
б) Заголовок Content-Type: appication/json.
в) Заголовок Content-Length: <размер тела ответа>.
г)Заголовок Cache-Control: no-cache.
д) Тело ответа — JSON-объект с полями code и message:
    1) Поле code должно иметь значение "invalidToken".
    2) Поле message — содержать понятное человеку описание ошибки.

Если заголовок Authorization содержит валидное значение токена, но в игре нет пользователя с таким токеном, должен вернуться ответ со следующими свойствами:
а) Статус-код 401 Unauthorized.
б) Заголовок Content-Type: appication/json.
в) Заголовок Content-Length: <размер тела ответа>.
г) Заголовок Cache-Control: no-cache.
д) Тело ответа — JSON-объект с полями code и message:
    1) Поле code должно иметь значение "unknownToken".
    2) Поле message — содержать понятное человеку описание ошибки.

/* ------------------------------------------------------------------------------------ */
**6) Реализовать в игровом сервере возможность конфигурировать скорость движения персонажей на карте

- Реализуйте возможность настраивать скорость персонажей.
Для этого в конфигурационный файл добавляются два новых свойства:
  1) Скорость персонажей на всех картах задаёт опциональное поле defaultDogSpeed в корневом JSON-объекте.
Если это поле отсутствует, скорость по умолчанию считается равной 1.
  2) Скорость персонажей на конкретной карте задаёт опциональное поле dogSpeed в соответствующем объекте карты.
Если это поле отсутствует, на карте используется скорость по умолчанию.

- Чтобы управлять действиями персонажа, реализуйте обработку POST-запроса к точке входа /api/v1/game/player/action.
Параметры запроса:
а) Обязательный заголовок Content-Type: application/json.
б) Обязательный заголовок Authorization: Bearer <токен пользователя> передаёт токен пользователя, полученный при входе в игру.
в) Тело запроса — JSON-объект с полем move, которое принимает одно из значений:
    1) "L" — задаёт направление движения персонажа влево (на запад).
    2) "R" — задаёт направление движения персонажа вправо (на восток).
    3) "U" — задаёт направление движения персонажа вверх (на север).
    4) "D" — задаёт направление движения персонажа вниз (на юг).
    5) "" — останавливает персонажа.
При обработке этих операций должна измениться скорость движения персонажа. Буквой s обозначена скорость персонажей на данной карте:
а) При движении влево скорость равна {-s, 0}.
б) При движении вправо скорость равна {s, 0}.
в) При движении вверх скорость равна {0, -s}.
г) При движении вниз скорость равна {0, s}.
д) При остановке скорость равна {0, 0}.

При успешной обработке запроса ответ на запрос должен обладать следующими свойствами:
а) Статус-код 200 OK.
б) Заголовок Cache-Control: no-cache.
в) Заголовок Content-Type: application/json.
г) Заголовок Content-Length: <размер тела ответа>.
д) Тело ответа — пустой JSON-объект.

Должна осуществляться проверка:
а) авторизации
б) JSON объекта в запросе
в) заголовка Content-Type
г) запрос только POST

В этом задании в игре время считается остановившимся. Поэтому объекты даже с  ненулевой скоростью не должны менять своих координат.
Получив команду управления движением, пес должен изменить значение скорости и направление движения, однако его координаты изменяться не должны.

/* ------------------------------------------------------------------------------------ */
**7) Реализовать в игровом сервере обновление игрового состояния с течением времени.

!!! Чтобы было легче протестировать вашу программу автотестами, временно доработайте код так,
чтобы после входа в игру пёс игрока появлялся в начальной точке первой дороги карты, а не в случайно сгенерированной.

!!! Добавить в REST API POST-запрос /api/v1/game/tick.
Тестирующая программа будет его отправлять, чтобы вручную контролировать ход времени сервера.

- Правила перемещения собак
1) Персонажи-собаки, управляемые игроками, могут перемещаться только вдоль дорог.
2) Ширина дороги равна 0.8 координатных единиц. Таким образом собака может удаляться от оси дороги не более, чем
на 0.4 координатные единицы.
3) За момент времени Δt персонаж, движущийся со скоростью V, должен переместиться на расстояние V * Δt, если нет препятствий.
4) Псам не разрешается перемещаться за пределами дорог.
Если вдоль вектора перемещения находится граница дороги, пёс должен остановиться у границы, и его скорость должна стать равна нулю.

* Чтобы быстро находить участок дороги по координатам, рекомендуем построить вспомогательную структуру данных, чтобы вместо линейного перебора всех дорог на карте искать дороги внутри map, unordered_map или отсортированного vector.

- Запрос для управления временем на карте
Добавьте в REST API POST-запрос для управления игровым временем к точке входа /api/v1/game/tick. Параметры запроса:
а) Обязательный заголовок Content-Type: application/json.
б) Тело запроса — JSON-объект с целочисленным полем timeDelta, задающим значение параметра Δt в миллисекундах.

При успешном выполнении этого запроса ответ сервера должен обладать следующими свойствами:
а) Статус код 200 OK.
б) Заголовок Cache-Control: no-cache.
в) Заголовок Content-Type: application/json.
г) Заголовок Content-Length: <размер тела ответа>.
д) Тело ответа — пустой JSON-объект.

После выполнения этого запроса все персонажи должны переместиться по правилам перемещения персонажей.
Последующие запросы игрового состояния должны возвращать новые координаты персонажей.

Если тело запроса содержит невалидный JSON, либо JSON-объект, поле timeDelta которого содержит невалидное значение, должен вернуться ответ со свойствами:
а) Статус-код 400 Bad request.
б) Заголовок Content-Type: application/json.
в) Заголовок Cache-Control: no-cache.
г) Заголовок Content-Length: <размер тела ответа>.
д) Тело ответа — JSON-объект с полями code и message:
    1) Поле code должно иметь значение "invalidArgument".
    2) Поле message — содержать понятное человеку описание ошибки.

/* ------------------------------------------------------------------------------------ */
**8) Реализовать в игровом сервере автоматическое обновление игрового состояния с течением времени и возможность запускать приложение в режиме тестирования.

- Примените Boost.ProgramOptions и обновите параметры командной строки приложения:
а) Параметр --tick-period (-t) задаёт период автоматического обновления игрового состояния в миллисекундах. Если этот параметр указан, каждые N миллисекунд сервер должен обновлять координаты объектов. Если этот параметр не указан, время в игре должно управляться с помощью запроса /api/v1/game/tick к REST API.
б) Параметр --config-file (-c) задаёт путь к конфигурационному JSON-файлу игры.
в) Параметр --www-root (-w) задаёт путь к каталогу со статическими файлами игры.
г) Параметр --randomize-spawn-points включает режим, при котором пёс игрока появляется в случайной точке случайно выбранной дороги карты.
д) Параметр --help (-h) должен выводить информацию о параметрах командной строки.

Если при запуске сервера параметр --randomize-spawn-points не указан, пёс игрока должен появляться в начальной точке первой дороги на карте.
Когда сервер запущен с параметром --tick-period, сервер должен отвечать на запросы к точке входа /api/v1/game/tick так же, как на запросы к любой другой несуществующей точке входа:
а) Статус-код 400 Bad request.
б) Заголовок Content-Type: application/json.
в) Заголовок Content-Length: <размер тела ответа>.
г) Заголовок Cache-Control: no-cache.
д) Тело запроса — JSON-объект с полями code и message:
    1) Поле code должно иметь значение "badRequest".
    2) Поле message — понятное человеку сообщение об ошибке.

- Реализовать режим реального времени, сам сервер должен с заданной периодичностью обновлять игровое время.