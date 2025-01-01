#pragma once

#include "character/character.h"
#include "character/character_reference.h"
#include "character/character_template.h"
#include "script/effect/effect.h"
#include "util/assert_util.h"
#include "util/string_util.h"
#include "util/string_conversion_util.h"

namespace kobold {

template <typename scope_type>
class create_character_effect final : public effect<scope_type>
{
public:
	explicit create_character_effect(const gsml_operator effect_operator)
		: effect<scope_type>(effect_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "create_character";
		return class_identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();

		if (key == "character_template") {
			this->character_template = character_template::get(value);
		} else if (key == "saved_scope") {
			this->saved_scope_name = value;
		} else if (key == "temporary") {
			this->temporary = string::to_bool(value);
		} else {
			effect<scope_type>::process_gsml_property(property);
		}
	}

	virtual void check() const override
	{
		assert_throw(this->character_template != nullptr);
		assert_throw(!this->saved_scope_name.empty());
	}

	virtual void do_assignment_effect(scope_type *scope, context &ctx) const override
	{
		Q_UNUSED(scope);

		const character *created_character = nullptr;

		if (this->temporary) {
			std::shared_ptr<character_reference> generated_character = character::generate_temporary(this->character_template, nullptr, nullptr, nullptr);
			created_character = generated_character->get_character();
			ctx.character_references.push_back(generated_character);
		} else {
			created_character = character::generate(this->character_template, nullptr, nullptr, nullptr, this->temporary);
		}

		ctx.get_saved_scopes<const character>()[this->saved_scope_name] = created_character;
	}

	virtual std::string get_assignment_string(scope_type *scope, const read_only_context &ctx, const size_t indent, const std::string &prefix) const override
	{
		Q_UNUSED(scope);
		Q_UNUSED(ctx);
		Q_UNUSED(indent);
		Q_UNUSED(prefix);

		return std::format("Create {} {}", string::get_indefinite_article(this->character_template->get_name()), string::highlight(this->character_template->get_name()));
	}

private:
	const kobold::character_template *character_template = nullptr;
	std::string saved_scope_name;
	bool temporary = true;
};

}
