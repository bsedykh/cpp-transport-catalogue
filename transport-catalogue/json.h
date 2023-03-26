#pragma once

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<nullptr_t, int, double, std::string, bool, Array, Dict>;

        Node() = default;
        Node(nullptr_t);
        Node(int value);
        Node(double value);
        Node(std::string value);
        Node(bool value);
        Node(Array array);
        Node(Dict map);
        Node(Value value);

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

    private:
        Value value_;

        static std::logic_error TypeMismatchException();
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
