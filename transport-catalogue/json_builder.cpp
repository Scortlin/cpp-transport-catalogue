#include "json_builder.h"
using namespace std;
namespace json{
KeyContext Builder::Key(string key){
    if (root_ == nullptr){
        throw logic_error("Key() called for empty document.");
    }
    else if (nodes_stack_.empty()){
        throw logic_error("Key() called outside of any container element.");
    }
    Node* parent_container = nodes_stack_.back();
    if (parent_container->IsDict()){
        if (!key_opened_){
            key_.swap(key);
            const_cast<Dict&>(parent_container->AsDict())[key_] = Node();
            key_opened_ = true;
        }
        else{
            throw logic_error("Key() called for a Dict with already setted Key. Should call Value()");
        }
    }
    else{
        throw logic_error("Key() called for a container other than Dict.");
    }
    return KeyContext{*this};
}

BaseContext Builder::Value(Node::Value val){
    if (root_ == nullptr){
        root_.GetValue() = std::move(val);
        return BaseContext{ *this };
    }
    if (nodes_stack_.empty()){
        throw logic_error("Value() called at wrong order.");
    }
    Node* current_container = nodes_stack_.back();
    if (current_container->IsArray()){
        const_cast<Array&>(current_container->AsArray()).push_back(move(val));
    }
    else if (current_container->IsDict()) {
        if (key_opened_){
            const_cast<Dict&>(current_container->AsDict())[key_] =move(val);
            key_.clear();
            key_opened_ = false;
        }
        else{
            throw logic_error("Value() called for Dict' Key field, not for Value field as intended.");
        }
    }
    else{
        throw logic_error("Value() called for unknown parent container.");
    }
    return BaseContext(*this);
}

DictItemContext Builder::StartDict(){
    if (root_ == nullptr){
        root_ = Dict();
        nodes_stack_.emplace_back(&root_);
        return DictItemContext{*this};
    }
    if (nodes_stack_.empty()){
        throw logic_error("StartDict() called at wrong order.");
    }
    Node* parent_container = nodes_stack_.back();
    if (parent_container->IsArray()){
        const_cast<Array&>(parent_container->AsArray()).push_back(Dict());
        Node* node = &const_cast<Array&>(parent_container->AsArray()).back();
        nodes_stack_.push_back(node);
    }
    else if (parent_container->IsDict()){
        if (key_opened_){
            const_cast<Dict&>(parent_container->AsDict())[key_] = Dict();
            Node* node = &const_cast<Dict&>(parent_container->AsDict()).at(key_);
            nodes_stack_.push_back(node);
            key_.clear();
            key_opened_ = false;
        }
        else{
            throw logic_error("StartDict() called for Key element, not for Value as intended.");
        }
    }
    else{
        throw logic_error("StartDict() called not for empty document, nor for Array element / Dict Value element.");
    }
    return DictItemContext{*this};
}

ArrayItemContext Builder::StartArray(){
    if (root_ == nullptr){
        root_ = Array();    
        nodes_stack_.emplace_back(&root_);
        return ArrayItemContext{*this};
    }
    if (nodes_stack_.empty()){
        throw logic_error("StartArray() called at wrong order.");
    }
    Node* parent_container = nodes_stack_.back();
    if (parent_container->IsArray()){
        const_cast<Array&>(parent_container->AsArray()).push_back(Array());
        Node* node = &const_cast<Array&>(parent_container->AsArray()).back();
        nodes_stack_.push_back(node);
    }
    else if (parent_container->IsDict()){
        if (key_opened_){
            const_cast<Dict&>(parent_container->AsDict())[key_] = Array();
            Node* node = &const_cast<Dict&>(parent_container->AsDict()).at(key_);
            nodes_stack_.push_back(node);
            key_.clear();
            key_opened_ = false;
        }
        else{
            throw logic_error("StartArray() called for Key element, not for Value.");
        }
    }
    else{
        throw logic_error("StartArray() called at wrong order.");
    }
    return ArrayItemContext{*this};
}

BaseContext Builder::EndDict(){
    if ((!nodes_stack_.empty()) && (nodes_stack_.back()->IsDict()) && (!key_opened_)){
        nodes_stack_.pop_back();
    }
    else{
        throw logic_error("EndDict() called at wrong order.");
    }
    return BaseContext{*this};
}

BaseContext Builder::EndArray(){
    if ((!nodes_stack_.empty()) && (nodes_stack_.back()->IsArray())){
        nodes_stack_.pop_back();
    }
    else{
        throw logic_error("EndArray() called at wrong order.");
    }
    return BaseContext{*this};
}

json::Node Builder::Build(){
    if (root_ == nullptr){
        throw logic_error("Build() called for empty document.");
    }
    else if (!nodes_stack_.empty()){
        throw logic_error("Build() called for not finished document.");
    }
    return std::move(root_);
}

KeyContext BaseContext::Key(string str){
    return builder_.Key(move(str));
}
BaseContext BaseContext::Value(Node::Value val){
    return builder_.Value(val);
}
DictItemContext BaseContext::StartDict(){
    return builder_.StartDict();
}
ArrayItemContext BaseContext::StartArray(){
    return builder_.StartArray();
}
BaseContext BaseContext::EndDict(){
    return builder_.EndDict();
}
BaseContext BaseContext::EndArray(){
    return builder_.EndArray();
}
json::Node BaseContext::Build(){
    return builder_.Build();
}

KeyValueContext KeyContext::Value(Node::Value val){
    return BaseContext::Value(std::move(val));
}
ArrayItemContext KeyContext::StartArray(){
    return BaseContext::StartArray();
}
DictItemContext KeyContext::StartDict(){
    return BaseContext::StartDict();
}


ArrayValueContext ArrayItemContext::Value(Node::Value val){
    return BaseContext::Value(std::move(val));
}
DictItemContext ArrayItemContext::StartDict(){
    return BaseContext::StartDict();
}
ArrayItemContext ArrayItemContext::StartArray(){
    return BaseContext::StartArray();
}

KeyContext DictItemContext::Key(string key){
    return BaseContext::Key(key);
}

KeyContext KeyValueContext::Key(string key){
    return BaseContext::Key(key);
}

ArrayValueContext ArrayValueContext::Value(Node::Value val){
    return BaseContext::Value(std::move(val));
}
DictItemContext ArrayValueContext::StartDict(){
    return BaseContext::StartDict();
}
ArrayItemContext ArrayValueContext::StartArray(){
    return BaseContext::StartArray();
}
BaseContext ArrayValueContext::EndArray(){
    return BaseContext::EndArray();
}

}