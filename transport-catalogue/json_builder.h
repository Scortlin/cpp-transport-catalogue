#pragma once

#include <string>
#include <stack>
#include <vector>
#include "json.h"

namespace json {
	class DictItemContext;
	class ArrayItemContext;
	class KeyItemContext;
	class SingleValueItemContext;

	class Builder {
	private:
		Node root_;
		std::stack<Node*> queue_;
	public:
		SingleValueItemContext Value(Node::Value value);
		Builder() = default;
		KeyItemContext Key(const std::string& key);
		DictItemContext StartDict();
		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
		Node Build();
	};

	class ItemContext {
	protected:
		Builder& builder_;
	public:
		SingleValueItemContext Value(Node::Value value);
		ItemContext(Builder& builder);
		KeyItemContext Key(const std::string& key);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node Build();
	};

	class KeyItemContext : public ItemContext {
	public:
		KeyItemContext Key(const std::string& key) = delete;
		KeyItemContext(Builder& builder);
		DictItemContext Value(Node::Value value);
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	};

	class SingleValueItemContext : public ItemContext {
	public:
		SingleValueItemContext Value(Node::Value value) = delete;
		SingleValueItemContext(Builder& builder);
		KeyItemContext Key(const std::string& key) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndDict() = delete;
	};

	class DictItemContext : public ItemContext {
	public:
		SingleValueItemContext Value(Node::Value value) = delete;
		DictItemContext(Builder& builder);
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	};

	class ArrayItemContext : public ItemContext {
	public:
		KeyItemContext Key(const std::string& key) = delete;
		ArrayItemContext(Builder& builder);
		ArrayItemContext Value(Node::Value value);
		Builder& EndDict() = delete;
		Node Build() = delete;
	};
}
