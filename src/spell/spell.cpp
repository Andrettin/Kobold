#include "kobold.h"

#include "spell/spell.h"

#include "character/character_class.h"
#include "spell/spell_effect.h"
#include "spell/spell_target.h"
#include "util/assert_util.h"
#include "util/vector_util.h"

namespace kobold {

spell::spell(const std::string &identifier)
	: named_data_entry(identifier), target(spell_target::none)
{
}

spell::~spell()
{
}

void spell::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "character_classes") {
		for (const std::string &value : values) {
			this->character_classes.push_back(character_class::get(value));
		}
	} else if (tag == "effects") {
		scope.for_each_element([&](const gsml_property &property) {
			auto effect = spell_effect::from_gsml_property(property);
			this->effects.push_back(std::move(effect));
		}, [&](const gsml_data &child_scope) {
			auto effect = spell_effect::from_gsml_scope(child_scope);
			this->effects.push_back(std::move(effect));
		});
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void spell::check() const
{
	assert_throw(this->get_target() != spell_target::none);
	assert_throw(this->get_icon() != nullptr);
	assert_throw(this->get_mana_cost() > 0);
	//assert_throw(!this->get_character_classes().empty());
	assert_throw(!this->effects.empty());
}

bool spell::is_available_for_character_class(const character_class *character_class) const
{
	return vector::contains(this->get_character_classes(), character_class);
}

}
