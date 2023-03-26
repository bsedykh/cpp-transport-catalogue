#include "json.h"

#include <cctype>
#include <utility>

using namespace std::literals;

namespace json {

    namespace {

        // ----------------- Print -----------------

        void PrintNode(const Node& node, std::ostream& output, int indent);

        std::string Indent(int indent) {
            std::string indent_str;
            for (int i = 0; i < indent; ++i) {
                indent_str += "    "s;
            }
            return indent_str;
        }

        void PrintArray(const Array& array, std::ostream& output, int indent) {
            output << "[\n"s;

            for (size_t i = 0; i < array.size(); ++i) {
                output << Indent(indent + 1);
                PrintNode(array[i], output, indent + 1);
                if (i < array.size() - 1) {
                    output << ',';
                }
                output << '\n';
            }

            output << Indent(indent) << ']';
        }

        void PrintMap(const Dict& dict, std::ostream& output, int indent) {
            output << "{\n"s;

            size_t i = 0;
            for (const auto& [key, value] : dict) {
                output << Indent(indent + 1) << '\"' << key << "\": ";
                PrintNode(value, output, indent + 1);
                if (i < dict.size() - 1) {
                    output << ',';
                }
                output << '\n';
                ++i;
            }

            output << Indent(indent) << '}';
        }

        void PrintString(const std::string& str, std::ostream& output) {
            output << '\"';
            for (char c : str) {
                switch (c) {
                case '\r':
                    output << "\\r";
                    break;
                case '\n':
                    output << "\\n";
                    break;
                case '\"':
                    output << "\\\"";
                    break;
                case '\\':
                    output << "\\\\";
                    break;
                default:
                    output << c;
                }
            }
            output << '\"';
        }

        void PrintNode(const Node& node, std::ostream& output, int indent) {
            if (node.IsArray()) {
                PrintArray(node.AsArray(), output, indent);
            }
            else if (node.IsMap()) {
                PrintMap(node.AsMap(), output, indent);
            }
            else if (node.IsString()) {
                PrintString(node.AsString(), output);
            }
            else if (node.IsNull()) {
                output << "null";
            }
            else if (node.IsInt()) {
                output << node.AsInt();
            }
            else if (node.IsPureDouble()) {
                output << node.AsDouble();
            }
            else if (node.IsBool()) {
                output << std::boolalpha << node.AsBool() << std::noboolalpha;
            }
        }

        // ----------------- Load -----------------

        void MoveToToken(std::istream& input) {
            auto read_char = [&input] {
                char c = static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                return c;
            };

            char c = ' ';
            while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                c = read_char();
                if (c == '\\') {
                    c = read_char();
                    switch (c) {
                    case 't':
                        c = '\t';
                        break;
                    case 'n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    default:
                        input.putback(c);
                    }
                }
            }

            input.putback(c);
        }

        Node LoadNode(std::istream& input);

        Node LoadNumber(std::istream& input) {
            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        // предполагается, что 1-я буква 'n' уже успешно прочитана
        Node LoadNull(std::istream& input) {
            std::string null_str;
            for (int i = 0; i < 3; ++i) {
                null_str += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read null from stream"s);
                }
            }

            if (null_str != "ull") {
                throw ParsingError("Invalid null token"s);
            }

            return Node();
        }

        // предполагается, что можно корректно прочитать 1-ю букву, и она равна 't' или 'f'
        Node LoadBool(std::istream& input) {
            std::string bool_str;

            const auto read_chars = [&bool_str, &input](int n) {
                for (int i = 0; i < n; ++i) {
                    bool_str += static_cast<char>(input.get());
                    if (!input) {
                        throw ParsingError("Failed to read bool from stream"s);
                    }
                }
            };

            bool_str += static_cast<char>(input.get());
            if (bool_str == "t"s) {
                //пробуем считать "rue"
                read_chars(3);
                if (bool_str != "true"s) {
                    throw ParsingError("Invalid bool token"s);
                }
                return Node(true);

            }
            else {
                //пробуем считать "alse"
                read_chars(4);
                if (bool_str != "false"s) {
                    throw ParsingError("Invalid bool token"s);
                }
                return Node(false);
            }
        }

        // символ '[' уже прочитан
        Node LoadArray(std::istream& input) {
            Array result;

            MoveToToken(input);
            char c = static_cast<char>(input.get());
            if (c == ']') {
                return Node(std::move(result));
            }

            input.putback(c);

            while (c != ']') {
                result.push_back(LoadNode(input));

                MoveToToken(input);
                c = static_cast<char>(input.get());
                if (!(c == ',' || c == ']')) {
                    throw ParsingError("Failed to load array: ',' or ']' is missing"s);
                }
            }

            return Node(std::move(result));
        }

        // символ '{' уже прочитан
        Node LoadDict(std::istream& input) {
            Dict result;

            MoveToToken(input);
            char c = static_cast<char>(input.get());
            if (c == '}') {
                return Node(std::move(result));
            }

            input.putback(c);

            while (c != '}') {
                auto node_key = LoadNode(input);
                if (!node_key.IsString()) {
                    throw ParsingError("Failed to load dict key"s);
                }

                MoveToToken(input);
                c = static_cast<char>(input.get());
                if (c != ':') {
                    throw ParsingError("Failed to load dict: ':' is missing"s);
                }

                Node value = LoadNode(input);

                result.emplace(node_key.AsString(), std::move(value));

                MoveToToken(input);
                c = static_cast<char>(input.get());
                if (!(c == ',' || c == '}')) {
                    throw ParsingError("Failed to load dict: ',' or '}' is missing"s);
                }
            }

            return Node(std::move(result));
        }

        Node LoadNode(std::istream& input) {
            MoveToToken(input);
            char c = static_cast<char>(input.get());

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == '-' || std::isdigit(c)) {
                input.putback(c);
                return LoadNumber(input);
            }
            else if (c == 'n') {
                return LoadNull(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else {
                throw ParsingError("Unknown token"s);
            }
        }

    }  // namespace

    Node::Node(nullptr_t) {}
    Node::Node(int value) : value_(value) {}
    Node::Node(double value) : value_(value) {}
    Node::Node(std::string value) : value_(std::move(value)) {}
    Node::Node(bool value) : value_(value) {}
    Node::Node(Array array) : value_(std::move(array)) {}
    Node::Node(Dict map) : value_(std::move(map)) {}
    Node::Node(Value value) : value_(std::move(value)) {}

    bool Node::IsNull() const {
        return std::holds_alternative<nullptr_t>(value_);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    std::logic_error Node::TypeMismatchException() {
        return std::logic_error("Type mismatch"s);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw TypeMismatchException();
        }
        return std::get<int>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw TypeMismatchException();
        }
        return std::get<bool>(value_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw TypeMismatchException();
        }

        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        else {
            return AsInt();
        }
    }

    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw TypeMismatchException();
        }
        return std::get<std::string>(value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw TypeMismatchException();
        }
        return std::get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw TypeMismatchException();
        }
        return std::get<Dict>(value_);
    }

    bool Node::operator==(const Node& other) const {
        return value_ == other.value_;
    }

    bool Node::operator!=(const Node& other) const {
        return !(*this == other);
    }

    Document::Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& other) const {
        return root_ == other.root_;
    }

    bool Document::operator!=(const Document& other) const {
        return root_ != other.root_;
    }

    Document Load(std::istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output, 0);
    }

}  // namespace json
