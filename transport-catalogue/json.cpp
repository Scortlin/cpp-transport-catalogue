#include "json.h"

using namespace std;

namespace json {
    namespace {
        using Number = variant<nullptr_t, bool, int, double>;
        Node LoadNode(istream& input);
        Node LoadArray(istream& input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') {
                throw ParsingError("Array parsing error");
            }
            return Node(move(result));
        }
        Node LoadString(std::istream& input) {
            using namespace std::literals;
            auto it = istreambuf_iterator<char>(input);
            auto end = istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
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
                        throw ParsingError("Unrecognized sequence"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(move(s));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c != '}') {
                throw ParsingError("Map parsing error");
            }
            return Node(move(result));
        }

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;
            string parsed_num;
            if (input.peek() == 'n') {
                string str;
                for (int i = 0; i < 4; ++i) {
                    str.push_back(static_cast<char>(input.get()));
                }
                if (str != "null") {
                    throw ParsingError("Expected null");
                }
                return nullptr;
            }

            if (input.peek() == 't') {
                std::string str;
                for (int i = 0; i < 4; ++i) {
                    str.push_back(static_cast<char>(input.get()));
                }
                if (str != "true") {
                    throw ParsingError("Expected boolean true");
                }
                return true;
            }

            if (input.peek() == 'f') {
                std::string str;
                for (int i = 0; i < 5; ++i) {
                    str.push_back(static_cast<char>(input.get()));
                }
                if (str != "false") {
                    throw ParsingError("Expected boolean false");
                }
                return false;
            }

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };
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
            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }
            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }
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
                    try {
                        return stoi(parsed_num);
                    }
                    catch (...) {}
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadInt(istream& input) {
            Number result = LoadNumber(input);
            if (holds_alternative<int>(result)) {
                return Node(get<int>(result));
            }

            if (holds_alternative<double>(result)) {
                return Node(get<double>(result));
            }

            if (holds_alternative<bool>(result)) {
                return Node(get<bool>(result));
            }
            return Node(nullptr);
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;
            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else {
                input.putback(c);
                return LoadInt(input);
            }
        }
    }

    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        if (holds_alternative<double>(*this)) {
            return true;
        }
        return holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return holds_alternative<string>(*this);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("It is not Array");
        }
        return get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw logic_error("It is not Map");
        }
        return get<Dict>(*this);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("It is not Integer");
        }
        return get<int>(*this);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("It is not string");
        }
        return get<std::string>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("It is not boolean");
        }
        return get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (!IsPureDouble() && !IsInt()) {
            throw logic_error("It is not dooble");
        }

        if (IsPureDouble()) {
            return std::get<double>(*this);
        }

        return std::get<int>(*this);
    }

    bool Node::Swap(Value&& val) {
        this->swap(val);
        return true;
    }

    bool Node::AddValue(Value&& val) {
        if (IsMap()) {
            return false;
        }
        if (IsArray()) {
            Array& itemAr = std::get<Array>(*this);
            Node newNode;
            newNode.Swap(move(val));
            itemAr.push_back(move(newNode));
            return true;
        }
        this->swap(val);
        return true;
    }

    bool Node::AddValue(std::string key, Value&& val) {
        if (!IsMap()) {
            return false;
        }
        Dict& itemDict = get<Dict>(*this);
        Node newNode;
        newNode.Swap(move(val));
        itemDict[key] = std::move(newNode);
        return true;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }
    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }
    void PrintArray(const Array& container, std::ostream& output) {
        output << "[";
        bool first = true;
        for (const Node& node : container) {
            if (!first) {
                output << ", ";
            }
            PrintNode(node, output);
            first = false;
        }
        output << "]";
    }

    void PrintMap(const Dict& container, std::ostream& output) {
        output << "{";
        bool first = true;
        for (const auto& [key, value] : container) {
            if (!first) {
                output << ", ";
            }
            output << "\"" << key << "\" : ";
            PrintNode(value, output);
            first = false;
        }
        output << "}";
    }

    void PrintString(const std::string& str, std::ostream& output) {
        std::unordered_set<char> specseq{ '\\', '\'', '"' };
        for (const char ch : str) {
            if (ch == '\r') {
                output << "\\r";
                continue;
            }
            if (ch == '\n') {
                output << "\\n";
                continue;
            }
            if (specseq.count(ch)) {
                output << '\\';
            }
            output << ch;
        }
    }

    void PrintNode(const Node& root, std::ostream& output) {
        if (root.IsInt()) {
            output << root.AsInt();
        }
        else if (root.IsDouble()) {
            output << root.AsDouble();
        }
        else if (root.IsString()) {
            output << "\"";
            PrintString(root.AsString(), output);
            output << "\"";
        }
        else if (root.IsNull()) {
            output << "null";
        }
        else if (root.IsBool()) {
            if (root.AsBool()) {
                output << "true";
            }
            else {
                output << "false";
            }
        }
        else if (root.IsArray()) {
            PrintArray(root.AsArray(), output);
        }
        else if (root.IsMap()) {
            PrintMap(root.AsMap(), output);
        }
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }
}