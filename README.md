# cpp-transport-catalogue
Финальный проект: транспортный справочник
# Функционал

1) Принимает на вход данные JSON-формата и выдает ответ в виде SVG-файла, визуализирующего остановки и маршруты.
2) Находит кратчайший маршрут между остановками.
3) Для ускорения вычислений сделана сериализация базы справочника через Google Protobuf.
4) Реализован конструктор JSON, позволяющий находить неправильную последовательность методов на этапе компиляции.

# Инструкция по сборке проекта (Visual Studio)
1) Установить Google Protobuf. Скачать с официального репозитория архив protobuf-cpp и распаковать его на компьютере.
2) Создать папки build-debug и build-release для сборки двух конфигураций Protobuf. Предварительно создать папку package, в которую будет складываться пакет Protobuf.
3) Собрать и установить проект (в примере сборка Debug) следующими командами:
```
cmake path\protobuf-3.15.8\cmake -DCMAKE_SYSTEM_VERSION=10.0.17763.0 -DCMAKE_BUILD_TYPE=Debug \ -Dprotobuf_BUILD_TESTS=OFF 
\ -DCMAKE_INSTALL_PREFIX=path\protobuf-3.15.8\package
cmake --build .
cmake --install .
```
4) В папке package появился bin\protoc.exe - с помощью него будут копилироваться proto-файлы, а в папке lib - статические библиотеки для работы с Protobuf.
5) Для компиляции proto-файла нужно выполнить следующую команду:
<путь к пакету Protobuf>\bin\proto --cpp_out . transport_catalogue.proto
6) Собрать проект с помощью CMake:
cmake . -DCMAKE_PREFIX_PATH=/path/to/protobuf/package
cmake --build .
7) При необходимости добавить папки include и lib в дополнительные зависимости проекта - Additional Include Directories и Additional Dependencies.

# Формат входных данных

# Стек технологий
OOP: inheritance, abstract interfaces, final classes
Unordered map/set
STL smart pointers
std::variant and std:optional
JSON load / output
SVG image format embedded inside XML output
Curiously Recurring Template Pattern (CRTP)
Method chaining
Facade design pattern
C++17 with С++20 Ranges emulation
Directed Weighted Graph data structure for Router module
Google Protocol Buffers for data serialization
Static libraries .LIB/.A
CMake generated project and dependency files

# Системные требования
Компилятор С++ с поддержкой стандарта C++17 или новее
