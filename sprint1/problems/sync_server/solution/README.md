Соберите сервер, принимающий HTTP-запросы на порте 8080 с помощью библиотеки boost::beast.
Сервер должен выдавать ответ на:
1) GET-запрос в следующем виде:
- Статус-код ответа: 200 OK
- HTTP-заголовок Content-Type должен иметь значение text/html
- Тело ответа должно быть равно Hello, {target}, где {target} — содержимое параметра target запроса, из которого удалили ведущий символ /. Например, при запросе URL-а: http://127.0.0.1:8080/Ivan тело ответа должно быть равно Hello, Ivan.
- HTTP-заголовок Content-Length имеет значение, равное длине тела ответа в байтах. Например, для строки Hello, Ivan заголовок Content-Length должен иметь значение 11.
2) HEAD-запрос должен быть аналогичен ответу на запрос GET за исключением пустого тела ответа. Значение заголовка Content-Length также должно быть равно значению этого заголовка в соответствующем GET-запросе.
3) Ответ на запросы, отличные от GET и HEAD, должен иметь следующий вид:
- Статус-код ответа: 405 Method Not Allowed
- HTTP-заголовок Content-Type имеет значение text/html.
- HTTP-заголовок Allow имеет значение GET, HEAD
- Тело ответа: Invalid method.
- HTTP-заголовок Content-Length имеет значение, равное длине тела ответа в байтах. Для тела Invalid method оно должно быть равно 14.