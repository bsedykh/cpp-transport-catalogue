# Транспортный справочник
В рамках проекта реализована система хранения транспортных маршрутов и обработка запросов к ней. Программа работает в два этапа: сначала на вход подаются запросы на создание базы данных, затем — запросы к самой базе.

Входные запросы содержат описания остановок и маршрутов. Каждая остановка задается географическими координатами и реальным (измеренным по дорогам) расстоянием до соседних остановок. Маршруты задаются последовательностью остановок.

Запросы на получение информации из базы данных бывают двух видов: запрос на получение информации о маршруте или об остановке. В первом случае выводится информация о маршруте: количество остановок всего, количество уникальных остановок, длина маршрута, извилистость[^1]. При запросе информации об остановке выводится список маршрутов, проходящих через остановку.

[^1]: Отношение фактической длины маршрута к географическому расстоянию. Равна единице в случае, когда автобус едет между остановками по кратчайшему пути.

## Пример использования
Подаем на вход запросы на создание базы данных:
```
13
Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino
Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino
Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka
Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino
Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
Stop Biryusinka: 55.581065, 37.64839, 750m to Universam
Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya
Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya
Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye
Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye
Stop Rossoshanskaya ulitsa: 55.595579, 37.605757
Stop Prazhskaya: 55.611678, 37.603831
```
Далее подаем на вход запросы информации о маршрутах и остановках:
```
6
Bus 256
Bus 750
Bus 751
Stop Samara
Stop Prazhskaya
Stop Biryulyovo Zapadnoye
```
Получаем вывод:
```
Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature
Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature
Bus 751: not found
Stop Samara: not found
Stop Prazhskaya: no buses
Stop Biryulyovo Zapadnoye: buses 256 828
```

## Используемые технологии
- С++17
- STL

## Описание исходных файлов
- transport_catalogue.h, transport_catalogue.cpp: содержит класс Catalogue, реализующий функционал транспортного справочника.
- input_reader.h, input_reader.cpp: содержит класс Reader для парсинга входных запросов на создание базы данных.
- stat_reader.h, stat_reader.cpp: содержит класс Reader для парсинга входных запросов на чтение информации из базы данных, а также, операторы для форматированного вывода результатов, полученных от класса Catalogue.
- geo.h: содержит вспомогательные функции и структуры данных для работы с географческими координатами.
- main.cpp: считывает входные запросы и выводит результат.

## Планы по доработке
- Добавить возможность визуализации карты маршрутов.
- Добавить возможность ввода / вывода в JSON.
- Добавить сборку с помощью CMake.
