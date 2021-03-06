pegjs-c-plugin
==============

Плагин для библиотеки [PEGjs](http://pegjs.org/), позволяющий сгенерировать исходный код на
C89 (также известном, как ANSI C).

API парсера
-----------
На вход подается диапазон -- массив байт для разбора. На выходе -- древовидная структура `Result`.
После окончания работы с данной структурой, надо освободить выделенную под нее память, вызвав
`freeResult`.

Структура `Result` содержит следующую информацию:

* Регион разбираемых данных, для которого была сформирована данная структура, в том числе указатель
  на начало и конец данных, а также информацию о строке и стобце для обоих концов (еще не реализовано).
* Количество дочерных элементов. Все дочерние элементы охватывают регионы, которые лежат внутри
  родительского региона.
* Указатель на пользовательские данные. Действия могут прикреплять туда свои данные во время разбора.
  Парсер не следит за их содержимым.

Ограничения
-----------
В отличие от оригинала, в Си нет автоматического управления памятью, и строгая типизация,
поэтому некоторые возможности не реализованы (но, возможно, будут реализованы в будущем):

* Функции, генерируемые для действий, возвращают `void`. Действия не могут поменять результат
  разбора, поэтому используйте их только для того, чтобы что-то сделать в процессе генерации
  над внешними структурами. Внешнюю структуру можно получить так: `context->options`.
* Семантические предикаты (`!{...}` и `&{...}`) должны возвращать `0` вместо false и любое
  ненулевое значение вместо `true`. Генерируемые функции возвращают значения типа `int`.
* Все параметры, передаваемые в действия/предикаты имеют тип `struct Result*`, объявленный в
  файле `peg.h`. Так как структуры данного типа выделяются в динамической области памяти, а
  парсер в любой момент может начать backtracking, при котором выделенная под них память
  освобождается, то вы не должны сохранять эти указатели для использования где-либо за пределами
  действия/семантического предиката.
* Так как парсер работает не с текстом, а с массивом байт, то регистронезависимые литералы/классы
  символов не поддерживаются. Флаг регистрозависимости игнорируется, все литералы/классы символов
  считаются регистрозависимыми.
* Как следствие предыдущего пункта, работа с юникодными символами не поддерживается. Если такой
  символ будет встречен в литерале/классе символов, то как он обработается, непонятно (не проверял).

Дополнительные возможности
--------------------------
Так как плагин писался под мою нужды, он поддерживает несколько возможностей, которые я оформил,
как pull-request-ы в оригинальный репозитарий. Все они доступны в [моем форке][1] в ветке [trunk][2].
Еще не все реализовано, но будет (реализованное отмечено галочкой):

- [x] [Диапазоны][3]
- [ ] Диапазоны, [разделители][4]
- [ ] Namespace-ы для кода, нужно для поддержки импорта

[1]: https://github.com/Mingun/pegjs
[2]: https://github.com/Mingun/pegjs/tree/trunk
[3]: https://github.com/pegjs/pegjs/pull/265
[4]: https://github.com/pegjs/pegjs/pull/267
