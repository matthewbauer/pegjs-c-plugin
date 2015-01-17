/** @file Содержит описание API генерируемых парсеров.
    Данный файл долен быть С89-совместимым, так, чтобы `gcc -pedantic -Wall -Wextra -ansi -std=c89`
    не выдавал предупреждений.
*/

#ifndef PARSER_API
#  define PARSER_API
#endif
#ifndef INTERNAL
#  define INTERNAL
#endif

/* Для free. */
#include <stdlib.h>
#include <assert.h>

/** Проверяет результат разбора на неудачу сопоставления. */
#define isFailed(r) ((r) == &FAILED)
#define isNil(r)    ((r) == &NIL)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Структуры парсера. */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/** Диапазон разбираемых элементов. */
struct Range {
  const char* begin;
  const char* end;
};
/** Содержит позицию в разбираемых данных, как в виде смещения в разбираемых элементах
    от начала строки, так и в виде номеров строки и столбца. Кроме того, содержит указатель
    на первый элемент разбираемых данных, с которого и начинается данная локация.
*/
struct Location {
  /** Данные, начинающиеся в этой локации. */
  const char* data;
  /** Смещение в элементах от начала разбираемых данных, нумерация с 0. */
  unsigned int offset;
  /** Номер строки в разбираемых данных, нумерация с 1. Новая строка начинается после символа
      '\r', '\n' или пары символов '\r\n'.
  */
  unsigned int line;
  /** Номер столбца в строке разбираемых данных, нумерация с 1. */
  unsigned int column;
};
struct Region {
  /** Начало региона с данными. */
  struct Location begin;
  /** Конец региона с данными. Не является частью региона, лежит за его концом. */
  struct Location end;
};
/** Узел дерева иерархического результата разбора. */
struct Result {
  /** Границы данного узла во входном потоке. */
  struct Region region;
  /** Пользовательские данные, привязанные к данному узлу. */
  void* userData;
  /** Количество потомков данного узла. */
  unsigned int count;
  /** Указатель на потомков данного узла. */
  struct Result** childs;
};
enum E_EXPECTED_TYPE {
  /** Ожидается любой символ. */
  E_EX_TYPE_ANY,
  /** Ожидается класс символов. */
  E_EX_TYPE_CLASS,
  /** Ожидается указанная последовательность символов. */
  E_EX_TYPE_LITERAL,
  /** Ожидается окончание потока данных. */
  E_EX_TYPE_EOF,
  /** Пользовательское сообщение об ожидаемых данных. */
  E_EX_TYPE_USER
};
struct Expected {
  unsigned int typeAndLen;
  const char* message;
};
struct FailInfo {
  unsigned int silent;
  /** Позиция, в которой необходимо сообщить о невозможности разбора. */
  struct Location pos;
  /** Количество элементов Expected в массиве expected. */
  unsigned int count;
  /** Массив возможных ожидаемых значений в позиции failPos. */
  const struct Expected** expected;
};
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Вспомогательные структуры. */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/** Тип функции для очистки пользовательских данных, когда структура результата разрушается. */
typedef void (*FreeUserDataFunc)(const void*);
struct Context {
  /** Разбираемые данные. */
  struct Range input;
  /** Текущая позиция в разбираемых данных. */
  struct Location current;
  /** Информация об ошибках разбора. */
  struct FailInfo failInfo;
  /** Функция для удаления ассациированных с @link Result результатами разбора@endlink
      пользовательских данных. В нее может приходить `NULL`. Данная функция вызывается
      перед удалением потомков узла результата, так что она может получить к ним доступ.
  */
  FreeUserDataFunc freeUserDataFunc;
  /** Информация, передаваемая пользователем в парсер. Парсером не используется. */
  void* userData;
};

/** Константа, возвращаемая из функций разбора в том случае, если разбор был неуспешен.
    Соответствие результата разбора данной константе может быть проверено макросом isFailed.
*/
static struct Result FAILED = {{{0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0, 0};
/** Константа, используемая как результат успешного разбора для предикатов
    и опциональных элементов, когда опциональное значение отсутствует.
    Соответствие результата разбора данной константе может быть проверено макросом isNil.
*/
static struct Result NIL    = {{{0, 0, 0, 0}, {0, 0, 0, 0}}, 0, 0, 0};
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Работа с памятью. */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void freeResult(/*struct Context* context, */struct Result* result) {
  if (result == &FAILED || result == &NIL) {
    return;
  }
  if (result) {
    /* Чистим пользовательские данные. */
    /*if (context->freeUserDataFunc) {
      (*context->freeUserDataFunc)(result->userData);
      result->userData = 0;
    }*/
    /* Если есть потомки, чистим сначала их, а затем и массив, их хранящий. */
    if (result->childs != 0) {
      unsigned int i;
      /* Если есть потомки, их количество должно быть задано. */
      assert(result->count);
      for (i = 0; i < result->count; ++i) {
        freeResult(result->childs[i]);
      }
      free(result->childs);
      result->count = 0;
      result->childs = 0;
    }
    free(result);
  }
}
