#include "json_builder.h"

#include <stdexcept>
#include <utility>

namespace json {

	// ----------------- Builder helper classes -----------------

	BuilderItem::BuilderItem(Builder& builder)
		: builder_(builder) {
	}

	BuilderArray BuilderArray::StartArray() {
		builder_.StartArray();
		return BuilderArray(builder_);
	}

	Builder& BuilderArray::EndArray() {
		return builder_.EndArray();
	}

	BuilderArray BuilderArray::Value(Node::Value value) {
		builder_.Value(std::move(value));
		return BuilderArray(builder_);
	}

	BuilderDict BuilderArray::StartDict() {
		builder_.StartDict();
		return BuilderDict(builder_);
	}

	BuilderKey BuilderDict::Key(std::string key) {
		builder_.Key(std::move(key));
		return BuilderKey(builder_);
	}

	Builder& BuilderDict::EndDict() {
		return builder_.EndDict();
	}

	BuilderDict BuilderKey::Value(Node::Value value) {
		builder_.Value(std::move(value));
		return BuilderDict(builder_);
	}

	BuilderDict BuilderKey::StartDict() {
		builder_.StartDict();
		return BuilderDict(builder_);
	}

	BuilderArray BuilderKey::StartArray() {
		builder_.StartArray();
		return BuilderArray(builder_);
	}

	// ----------------- Builder -----------------

	BuilderKey Builder::Key(std::string key) {
		if (context_.empty() || context_.back().type != ContextType::DICT) {
			throw std::logic_error("Key error");
		}

		values_.emplace_back(std::move(key));
		context_.push_back({ ContextType::KEY, values_.size() - 1 });

		return BuilderKey(*this);
	}

	Builder& Builder::Value(Node::Value value) {
		if (!CanValue()) {
			throw std::logic_error("Value error");
		}

		values_.emplace_back(std::move(value));

		ExtractKeyIfNeeded();

		return *this;
	}

	BuilderDict Builder::StartDict() {
		if (!CanValue()) {
			throw std::logic_error("StartDict error");
		}
		context_.push_back({ ContextType::DICT, values_.size() });

		return BuilderDict(*this);
	}

	Builder& Builder::EndDict() {
		if (context_.empty() || context_.back().type != ContextType::DICT) {
			throw std::logic_error("EndDict error");
		}

		Dict dict;
		size_t index = context_.back().index;
		for (; index < values_.size(); ++index) {
			std::string key = values_[index++].AsString();
			Node value = std::move(values_[index]);
			dict.emplace(std::move(key), std::move(value));
		}
		ReplaceValue(std::move(dict));

		return *this;
	}

	BuilderArray Builder::StartArray() {
		if (!CanValue()) {
			throw std::logic_error("StartArray error");
		}
		context_.push_back({ ContextType::ARRAY, values_.size() });
		return BuilderArray(*this);
	}

	Builder& Builder::EndArray() {
		if (context_.empty() || context_.back().type != ContextType::ARRAY) {
			throw std::logic_error("EndArray error");
		}

		Array arr;
		size_t index = context_.back().index;
		for (; index < values_.size(); ++index) {
			arr.push_back(std::move(values_[index]));
		}
		ReplaceValue(std::move(arr));

		return *this;
	}

	Node Builder::Build() const {
		if (!context_.empty() || values_.empty()) {
			throw std::logic_error("Build error");
		}
		return values_.back();
	}

	// ----------------- Helper functions -----------------

	bool Builder::CanValue() const {
		return (context_.empty() && values_.empty())
			|| (!context_.empty() && context_.back().type != ContextType::DICT);
	}

	void Builder::ExtractKeyIfNeeded() {
		if (!context_.empty() && context_.back().type == ContextType::KEY) {
			context_.pop_back();
		}
	}

	void Builder::ReplaceValue(Node value) {
		values_.resize(context_.back().index);
		values_.push_back(std::move(value));
		context_.pop_back();
		ExtractKeyIfNeeded();
	}

} // namespace json