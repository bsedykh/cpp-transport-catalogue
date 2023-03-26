#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

	class Builder;
	class BuilderArray;
	class BuilderDict;
	class BuilderKey;

	class BuilderItem {
	public:
		BuilderItem(Builder& builder);

	protected:
		Builder& builder_;
	};

	class BuilderArray : public BuilderItem {
	public:
		using BuilderItem::BuilderItem;
		BuilderArray StartArray();
		Builder& EndArray();
		BuilderArray Value(Node::Value value);
		BuilderDict StartDict();
	};

	class BuilderDict : public BuilderItem {
	public:
		using BuilderItem::BuilderItem;
		BuilderKey Key(std::string key);
		Builder& EndDict();
	};

	class BuilderKey : public BuilderItem {
	public:
		using BuilderItem::BuilderItem;
		BuilderDict Value(Node::Value value);
		BuilderDict StartDict();
		BuilderArray StartArray();
	};

	class Builder {
	public:
		BuilderKey Key(std::string key);
		Builder& Value(Node::Value value);
		BuilderDict StartDict();
		Builder& EndDict();
		BuilderArray StartArray();
		Builder& EndArray();
		Node Build() const;

	private:
		enum class ContextType {
			ARRAY, DICT, KEY
		};

		struct Context {
			ContextType type;
			size_t index;
		};

		std::vector<Context> context_;
		std::vector<Node> values_;

		bool CanValue() const;
		void ExtractKeyIfNeeded();
		void ReplaceValue(Node value);
	};

} // namespace json