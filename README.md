# Введение в многопоточное программирование

## Задание 1. FindIf

Решение задания находится в ```find_if/find_if.h```.

### Условие
Написать функцию, которая будет находить все числа в диапазоне
от ```lower_bound```до ```upper_bound``` (включая границы),
обладающие некоторым свойством. Сигнатура:
```
std::vector<int64_t> FindIf(int64_t lower_bound, int64_t upper_bound,
                            const std::function<bool(int64_t)>& predicate,
                            uint8_t concurrency = 1);
```

### Решение
Однопоточное решение для каждого числа на отрезке ```[lower_bound; upper_bound]```
вызывает функцию-предикат, после чего, в зависимости от результата,
либо добавляет ответ в контейнер, либо пропускает его.

Многопоточное решение сперва разбивает отрезок на ```concurrency``` подотрезков, 
после чего выполняет ту же процедуру, что и однопоточное, только каждый
подотрезок обрабатывается в отдельном потоке. При обращении к общему контейнеру,
поток блокирует контейнер под себя, после чего добавляет значение.

### Время работы
Нетрудно понять, что время работы этих решений на одинаковых подотрезках зависит от
времени работы функции-предиката, а также от количества добавляемых ответов в контейнер
(то есть, от количества подходящих под предикат чисел).

**Чем больше чисел из отрезка подходит под предикат, тем больше будет получать
преимущество однопоточное решение**.

Преимущество вызвано тем, что добавление в контейнер может выполняться все равно
только одним потоком одновременно, а если несколько потоков пытаются добавить ответ, то
все, кроме одного, будут просто ждать.

А так как на блокировку mutex-ов также уходит время, то иногда однопоточное решение
может оказаться даже быстрее.

Также, **чем больше время работы функции-предиката, тем больше преимущества уже у
многопоточного варианта**.

Объясняется это тем, что предикаты, в отличие от добавления в контейнер, могут
проверяться абсолютно независимо. В связи с этим, одновременно можно проверять
```concurrency``` предикатов, что сэкономит многопоточному решению немало времени.

## Задание 2. Hash collision

Решение задания находится в ```hash_collision/hash.h```.

### Условие
Реализовать функцию
```
std::string FindCollision(const std::string& a, int64_t p, int64_t m, uint8_t concurrency)
```
которая принимает строку ```a``` и находит такую строку ```b```, что
* ```a != b```
* ```Hash(a) == Hash(b)```

При этом решение должно использовать ```concurrency``` потоков и работать быстрее,
чем однопоточное.

Для простоты все строки в данной задаче состоят из маленьких латинских символов.
Полиномиальный хеш от строки с параметрами p и m вычисляется следующей фукнцией:
```
int64_t Hash(const std::string& s, int64_t p, int64_t m) {
    int64_t result = 0;
    for (char ch : s) {
        result = (__int128_t(result) * p + (ch - 'a' + 1)) % m;
    }
    return result;
}
```

### Решение

Будем искать коллизии в строках небольшой длины, независимо от длины данной строки.
Заметим, что полиномиальный хеш обладает следующим свойством:
```
Hash(1...n) = (Hash(1...n/2) * p^{n/2} + Hash(n/2+1...n)) % m
```
где ```Hash(i...j)``` - хеш подстроки из символов на позициях ```[i; j]```.
Отсюда несложно выразить хеш второй части:
```
Hash(n/2+1...n) = (Hash(1...n) - Hash(1...n/2) * p^{n/2} + m) % m
```

Теперь достаточно генерировать строки длины в 2 раза меньше. Найдем всевозможные
строки длины ```n/2``` и соберем их в HashMap - это будут суффиксы искомой строки.

Далее будем также генерировать все строки длины ```n/2``` (префиксы), вычислять
по формуле выше требуемый хеш суффикса и искать его в HashMap. Если такой суффикс
уже есть, то ответ найдет. 

### Время работы

Длина искомой строки начинается с 1 и увеличивается до тех пор, пока коллизия
не будет найдена. Для модулей порядка 1е10 хватает строк длины 8 (генерировать
строки длины 4), а для модулей не более 1е13 -- длины 10.

Однопоточное решение работает в среднем около ```60мс``` на модуле порядка 1е9,
а на модуле порядка 1е13 - около ```3с```.

Многопоточное решение, например, с количеством потоков равным 8, работает на
модуле порядка 1е9 около ```30мс```, а на модуле порядка 1е13 - ```1с```.

Все измерения были произведены с помощью
<a href="https://github.com/google/benchmark">Google Benchmark</a>.
Чтобы запустить их локально, можно воспользоваться конфигурацией
```HashCollisionBench```, добавленной в ```CMakeLists.txt```.

## Задание 3. Unkept promises

Решение задания находится в ```unkept_promises/promise.h```.

### Условие
Реализовать класс ```Promise```, позволяющий запустить функцию на выполнение
в фоновом потоке, а затем получить её результат.

Реализовать шаблонный класс ```Promise<ResultType>``` со следующими методами:

* Конструктор, принимающий произвольную функцию и запускающий её на выполнение
в фоновом потоке. Вызов конструктора должен **сразу же завершаться**
(не блокироваться).

* Метод ```ResultType Wait()```, который дожидается выполнения функции, переданной
в конструкторе и возвращает её результат. 
Этот метод **не должен работать “активно”**, т.е. занимать процессорное время.

Дополнительно создать свободную функцию ```MakePromise```,
которая принимает функцию, определяет тип её результата и
создаёт ```Promise``` с нужным ```ResultType```.

* **Функции, переданные в конструктор, могут бросать исключения**.
При выбрасывании исключения из функции, переданной в ```std::thread```,
вызывается ```std::terminate``` и работа всех потоков прекращается.
Предлагается ловить исключения, выброшенные функцией и выбрасывать их из вызова
```Wait```, чтобы вызывающая сторона могла их обработать. При этом можно считать,
что все возможные исключения наследуются от ```std::exception```.

* Предлагается поддержать возможность **выстраивать цепочки из ```Promise```**.
Для этого нужно реализовать следующие методы:
    * ```Then```, принимающий функцию, которая умеет принимать ```ResultType```
    нашего ```Promise```. Метод ```Then``` должен вернуть ```Promise<NewResult>```
    и **сразу же завершиться**.
    * ```Catch(Handler)```, принимающий функцию, которая будет обрабатывать
    исключения, которые могли возникнуть в предыдущих обработчиках в цепочке.
    Если исключения не выбрасывались, то хендлер не вызывается.
    **Метод Catch не должен блокироваться**.

Цепочки будут иметь вид
```
auto promise = MakePromise(...)->Then(...)->Then(...)->Catch(...)->Then(...);
promise->Wait();
```
