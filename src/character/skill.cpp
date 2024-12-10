#include "kobold.h"

#include "character/skill.h"

#include "character/skill_category.h"

namespace kobold {

void skill::check() const
{
	if (this->get_attribute() == nullptr) {
		throw std::runtime_error(std::format("Skill \"{}\" has no attribute.", this->get_identifier()));
	}

	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Skill \"{}\" has no icon.", this->get_identifier()));
	}
}

void skill::set_category(skill_category *category)
{
	this->category = category;

	category->add_skill(this);
}

const character_attribute *skill::get_attribute() const
{
	if (this->attribute != nullptr) {
		return this->attribute;
	}

	if (this->get_category() != nullptr) {
		return this->get_category()->get_attribute();
	}

	return this->attribute;
}

const kobold::icon *skill::get_icon() const
{
	if (this->icon != nullptr) {
		return this->icon;
	}

	if (this->get_category() != nullptr) {
		return this->get_category()->get_icon();
	}

	return this->icon;
}

}
