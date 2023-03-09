#include "json.h"
#include <iterator>
using namespace std;
namespace json{



Node LoadNode(istream& input);
Node LoadString(istream& input);

string LoadLiteral(istream& input){
    string s;
    while (isalpha(input.peek())){
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}

Node LoadArray(istream& input){
    vector<Node> result;
    for (char c; input >> c && c != ']';){
        if (c != ','){
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input){
        throw ParsingError("Array parsing error"s);
    }
    return Node(move(result));
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

Node LoadString(istream& input){
    auto it = istreambuf_iterator<char>(input);
    auto end = istreambuf_iterator<char>();
    string s;
    while (true){
        if (it == end)
        {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"'){
            ++it;
            break;
        }
        else if (ch == '\\'){
            ++it;
            if (it == end){
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char){
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
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r'){
            throw ParsingError("Unexpected end of line"s);
        }
        else{
            s.push_back(ch);
        }
        ++it;
    }
    return Node(move(s));
}

Node LoadBool(istream& input)
{
    const auto s = LoadLiteral(input);
    if (s == "true"sv){
        return Node{ true };
    }
    else if (s == "false"sv){
        return Node{ false };
    }
    else{
        throw ParsingError("Failed to parse '"s + s + "' as bool"s);
    }
}

Node LoadNull(istream& input)
{
    if (auto literal = LoadLiteral(input); literal == "null"sv){
        return Node{ nullptr };
    }
    else{
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
    }
}

Node LoadNumber(istream& input)
{
    string parsed_num;
    auto read_char = [&parsed_num, &input]{
        parsed_num += static_cast<char>(input.get());
        if (!input){
            throw ParsingError("Failed to read number from stream"s);
        }
    };
    auto read_digits = [&input, read_char]{
        if (!isdigit(input.peek())){
            throw ParsingError("A digit is expected"s);
        }
        while (isdigit(input.peek())){
            read_char();
        }
    };

    if (input.peek() == '-'){
        read_char();
    }
    if (input.peek() == '0'){
        read_char();
    }
    else{
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.'){
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E'){
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-'){
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try{
        if (is_int){
            try{
                return stoi(parsed_num);
            }
            catch (...){}
        }
        return stod(parsed_num);
    }
    catch (...){
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(istream& input){
    char c;
    input >> c;
    switch (c){
    case '[':
        return LoadArray(input);
    case '{':
        return LoadDict(input);
    case '"':
        return LoadString(input);
    case 't':
        [[fallthrough]];
    case 'f':
        input.putback(c);
        return LoadBool(input);
    case 'n':
        input.putback(c);
        return LoadNull(input);
    default:
        input.putback(c);
        return LoadNumber(input);
    }
}

struct PrintContext{
    ostream& out;
    int indent_step = 4;
    int indent = 0;
    void PrintIndent() const{
        for (int i = 0; i < indent; ++i){
            out.put(' ');
        }
    }
    PrintContext Indented() const{
        return { out, indent_step, indent_step + indent };
    }
};

void PrintNode(const Node& value, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx){
    ctx.out << value;
}

void PrintString(const string& value, ostream& out){
    out.put('"');
    for (const char c : value){
        switch (c){
        case '\r':
            out << "\\r"sv;
            break;
        case '\n':
            out << "\\n"sv;
            break;
        case '"':
            [[fallthrough]];
        case '\\':
            out.put('\\');
            [[fallthrough]];
        default:
            out.put(c);
            break;
        }
    }
    out.put('"');
}

template <>
void PrintValue<string>(const std::string& value, const PrintContext& ctx){
    PrintString(value, ctx.out);
}

template <>
void PrintValue<nullptr_t>(const nullptr_t&, const PrintContext& ctx){
    ctx.out << "null"sv;
}

template <>
void PrintValue<bool>(const bool& value, const PrintContext& ctx){
    ctx.out << (value ? "true"sv : "false"sv);
}

template <>
void PrintValue<Array>(const Array& nodes, const PrintContext& ctx)
{
    ostream& out = ctx.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const Node& node : nodes){
        if (first){
            first = false;
        }
        else
        {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

template <>
void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx){
    ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes){
        if (first){
            first = false;
        }
        else{
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}
void Print(const Document& doc, ostream& output){
    PrintNode(doc.GetRoot(), PrintContext{ output });
}

void PrintNode(const Node& node, const PrintContext& ctx){
    visit([&ctx](const auto& value){PrintValue(value, ctx);},node.GetValue());
}

Document Load(istream& input){
    return Document{ LoadNode(input) };
}

Node::Node(Value value) : variant(std::move(value))
{}

}