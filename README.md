# Транспортный справочник
В рамках проекта реализована система хранения транспортных маршрутов и обработка запросов к ней. На вход подаются запросы двух видов: запросы на создание базы данных и запросы на чтение данных. Программа обрабатывает запросы первого вида и создает базу данных. После этого обрабатываются запросы на чтение данных и выводится результат. 

Запросы на создание базы данных содержат описания остановок и маршрутов. Каждая остановка задается географическими координатами и реальным (измеренным по дорогам) расстоянием до соседних остановок. Маршруты задаются последовательностью остановок.

Запросы на чтение данных бывают трех видов:
- Запрос на получение информации о маршруте. Выводится количество остановок всего, количество уникальных остановок, длина маршрута и его извилистость[^1].
- Запрос на получение информации об остановке. Выводится список маршрутов, проходящих через остановку.
- Запрос на построение карты маршрутов. Выводится документ в формате SVG, содержащий изображение карты маршрутов.

[^1]: Отношение фактической длины маршрута к географическому расстоянию. Равна единице в случае, когда автобус едет между остановками по кратчайшему пути.

Ввод-вывод осуществляется в формате JSON.

## Пример использования
На вход подаются запросы на создание базы данных (base_requests), настройки для рендеринга карты маршрутов (render_settings), запросы на чтение данных (stat_requests):
```json
{
    "base_requests": [
      {
        "type": "Bus",
        "name": "114",
        "stops": ["Морской вокзал", "Ривьерский мост"],
        "is_roundtrip": false
      },
      {
        "type": "Stop",
        "name": "Ривьерский мост",
        "latitude": 43.587795,
        "longitude": 39.716901,
        "road_distances": {"Морской вокзал": 850}
      },
      {
        "type": "Stop",
        "name": "Морской вокзал",
        "latitude": 43.581969,
        "longitude": 39.719848,
        "road_distances": {"Ривьерский мост": 850}
      }
    ],
    "render_settings": {
      "width": 200,
      "height": 200,
      "padding": 30,
      "stop_radius": 5,
      "line_width": 14,
      "bus_label_font_size": 20,
      "bus_label_offset": [7, 15],
      "stop_label_font_size": 20,
      "stop_label_offset": [7, -3],
      "underlayer_color": [255,255,255,0.85],
      "underlayer_width": 3,
      "color_palette": ["green", [255,160,0],"red"]
    },
    "stat_requests": [
      { "id": 1, "type": "Map" },
      { "id": 2, "type": "Stop", "name": "Ривьерский мост" },
      { "id": 3, "type": "Bus", "name": "114" }
    ]
  }
```

На выходе формируется массив, содержащий ответы на запросы stat_requests (в порядке следования запросов):
```json
[
    {
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">114</text>\n  <text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\">114</text>\n  <text x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">114</text>\n  <text x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\" fill=\"green\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">Морской вокзал</text>\n  <text x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"black\">Морской вокзал</text>\n  <text x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\">Ривьерский мост</text>\n  <text x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\" fill=\"black\">Ривьерский мост</text>\n</svg>",
        "request_id": 1
    },
    {
        "buses": [
            "114"
        ],
        "request_id": 2
    },
    {
        "curvature": 1.23199,
        "request_id": 3,
        "route_length": 1700,
        "stop_count": 3,
        "unique_stop_count": 2
    }
]
```
Полученный в первом ответе SVG документ отображает карту маршрутов:

![bus_map](https://user-images.githubusercontent.com/84812761/224555584-f056da79-5a4b-4fa3-b61a-350bb5a46ecd.svg)

## Используемые технологии
- С++17
- STL

## Описание исходных файлов
- geo.h, geo.cpp: работа с географческими координатами.
- json.h, json.cpp: библиотека для работы с JSON.
- json_reader.h, json_reader.cpp: чтение запросов из JSON, формирование массива JSON-ответов.
- main.cpp: чтение входных запросов из stdin и вывод результатов в stdout.
- map_renderer.h, map_renderer.cpp: рендеринг карты маршрутов.
- svg.h, svg.cpp: библиотека для работы с SVG.
- transport_catalogue.h, transport_catalogue.cpp: хранение списка маршрутов.

## Планы по доработке
- Добавить сборку с помощью CMake.
