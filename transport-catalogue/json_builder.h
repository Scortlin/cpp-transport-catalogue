#pragma once

#include "json.h"

namespace json{

class Builder;
class BaseContext;
class KeyContext;
class DictItemContext;
class ArrayItemContext;
class KeyValueContext;
class ArrayValueContext;

class Builder{
public:
    KeyContext Key(std::string);
    BaseContext Value(Node::Value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    json::Node Build();
private:
    Node root_ = nullptr;
    std::vector<Node*> nodes_stack_;
    std::string key_;
    bool key_opened_ = false;
};

class BaseContext{
public:
    BaseContext(Builder& builder) : builder_(builder)
    {}

    KeyContext Key(std::string);
    BaseContext Value(Node::Value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    json::Node Build();
private:
    Builder& builder_;
};

class KeyContext : public BaseContext{
public:
    KeyContext(Builder& builder) : BaseContext(builder)
    {}
    KeyValueContext Value(Node::Value);
    ArrayItemContext StartArray();
    DictItemContext StartDict();
    KeyContext Key(std::string) = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    json::Node Build() = delete;
};

class ArrayItemContext : public BaseContext{
public:
    ArrayItemContext(Builder& builder) : BaseContext(builder)
    {}
    ArrayValueContext Value(Node::Value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    KeyContext Key(std::string) = delete;
    BaseContext EndDict() = delete;
    json::Node Build() = delete;
};

class DictItemContext : public BaseContext{
public:
    DictItemContext(Builder& builder) : BaseContext(builder)
    {}
    KeyContext Key(std::string);
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseContext Value(Node::Value) = delete;
    BaseContext EndArray() = delete;
    json::Node Build() = delete;
};

class KeyValueContext : public BaseContext{
public:
    KeyValueContext(Builder& builder) : BaseContext(builder)
    {}
    KeyValueContext(BaseContext base_context) : BaseContext(base_context)
    {}

    KeyContext Key(std::string);
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseContext Value(Node::Value) = delete;
    BaseContext EndArray() = delete;
    json::Node Build() = delete;
};

class ArrayValueContext : public BaseContext{
public:
    ArrayValueContext(Builder& builder) : BaseContext(builder)
    {}
    ArrayValueContext(BaseContext base_context) : BaseContext(base_context)
    {}
    ArrayValueContext Value(Node::Value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndArray();
    BaseContext EndDict() = delete;
    KeyContext Key(std::string) = delete;
    json::Node Build() = delete;
};

}