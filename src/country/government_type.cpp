#include "kobold.h"

#include "country/government_type.h"

#include "character/character_class.h"
#include "character/feat_template.h"
#include "country/country_tier.h"
#include "country/government_group.h"
#include "country/office.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "util/assert_util.h"
#include "util/gender.h"

namespace kobold {

const std::set<std::string> government_type::database_dependencies = {
	feat_template::class_identifier
};

void government_type::process_title_name_scope(std::map<government_variant, title_name_map> &title_names, const gsml_data &scope)
{
	scope.for_each_property([&](const gsml_property &property) {
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();

		government_variant government_variant{};
		const government_group *government_group = government_group::try_get(key);
		if (government_group != nullptr) {
			government_variant = government_group;
		} else {
			government_variant = government_type::get(key);
		}

		title_names[government_variant][country_tier::none] = value;
	});

	scope.for_each_child([&](const gsml_data &child_scope) {
		government_variant government_variant{};
		const government_group *government_group = government_group::try_get(child_scope.get_tag());
		if (government_group != nullptr) {
			government_variant = government_group;
		} else {
			government_variant = government_type::get(child_scope.get_tag());
		}

		government_type::process_title_name_scope(title_names[government_variant], child_scope);
	});
}

void government_type::process_title_name_scope(title_name_map &title_names, const gsml_data &scope)
{
	scope.for_each_property([&](const gsml_property &property) {
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();
		const country_tier tier = enum_converter<country_tier>::to_enum(key);
		title_names[tier] = value;
	});
}

void government_type::process_office_title_name_scope(data_entry_map<office, std::map<government_variant, office_title_inner_name_map>> &office_title_names, const gsml_data &scope)
{
	scope.for_each_child([&](const gsml_data &child_scope) {
		const office *office = office::get(child_scope.get_tag());

		government_type::process_office_title_name_scope(office_title_names[office], child_scope);
	});
}

void government_type::process_office_title_name_scope(office_title_name_map &office_title_names, const gsml_data &scope)
{
	scope.for_each_child([&](const gsml_data &child_scope) {
		const office *office = office::get(child_scope.get_tag());

		government_type::process_office_title_name_scope(office_title_names[office], child_scope);
	});
}

void government_type::process_office_title_name_scope(std::map<government_variant, office_title_inner_name_map> &office_title_names, const gsml_data &scope)
{
	scope.for_each_child([&](const gsml_data &child_scope) {
		government_variant government_variant{};
		const government_group *government_group = government_group::try_get(child_scope.get_tag());
		if (government_group != nullptr) {
			government_variant = government_group;
		} else {
			government_variant = government_type::get(child_scope.get_tag());
		}

		government_type::process_office_title_name_scope(office_title_names[government_variant], child_scope);
	});
}

void government_type::process_office_title_name_scope(office_title_inner_name_map &office_title_names, const gsml_data &scope)
{
	scope.for_each_property([&](const gsml_property &property) {
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();
		if (enum_converter<country_tier>::has_value(key)) {
			const country_tier tier = enum_converter<country_tier>::to_enum(key);
			office_title_names[tier][gender::none] = value;
		} else {
			const gender gender = enum_converter<kobold::gender>::to_enum(key);
			office_title_names[country_tier::none][gender] = value;
		}
	});

	scope.for_each_child([&](const gsml_data &child_scope) {
		const country_tier tier = enum_converter<country_tier>::to_enum(child_scope.get_tag());

		government_type::process_office_title_name_scope(office_title_names[tier], child_scope);
	});
}

void government_type::process_office_title_name_scope(std::map<gender, std::string> &office_title_names, const gsml_data &scope)
{
	scope.for_each_property([&](const gsml_property &property) {
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();
		const gender gender = enum_converter<archimedes::gender>::to_enum(key);
		office_title_names[gender] = value;
	});
}

government_type::government_type(const std::string &identifier) : named_data_entry(identifier)
{
}

government_type::~government_type()
{
}

void government_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "conditions") {
		auto conditions = std::make_unique<and_condition<country>>();
		database::process_gsml_data(conditions, scope);
		this->conditions = std::move(conditions);
	} else if (tag == "modifier") {
		auto modifier = std::make_unique<kobold::modifier<const country>>();
		database::process_gsml_data(modifier, scope);
		this->modifier = std::move(modifier);
	} else if (tag == "effects") {
		auto effect_list = std::make_unique<kobold::effect_list<const country>>();
		database::process_gsml_data(effect_list, scope);
		this->effects = std::move(effect_list);
	} else if (tag == "ruler_character_classes") {
		for (const std::string &value : values) {
			this->ruler_character_classes.push_back(character_class::get(value));
		}
	} else if (tag == "title_names") {
		government_type::process_title_name_scope(this->title_names, scope);
	} else if (tag == "office_title_names") {
		government_type::process_office_title_name_scope(this->office_title_names, scope);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void government_type::check() const
{
	if (this->get_group() == nullptr) {
		throw std::runtime_error(std::format("Government type \"{}\" has no government group.", this->get_identifier()));
	}

	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Government type \"{}\" has no icon.", this->get_identifier()));
	}

	if (this->get_conditions() != nullptr) {
		this->get_conditions()->check_validity();
	}

	if (this->get_ruler_character_classes().empty()) {
		throw std::runtime_error(std::format("Government type \"{}\" has no ruler character classes.", this->get_identifier()));
	}
}

const std::string &government_type::get_title_name(const country_tier tier) const
{
	const auto find_iterator = this->title_names.find(tier);
	if (find_iterator != this->title_names.end()) {
		return find_iterator->second;
	}

	return this->get_group()->get_title_name(tier);
}

const std::string &government_type::get_office_title_name(const office *office, const country_tier tier, const gender gender) const
{
	const auto office_find_iterator = this->office_title_names.find(office);
	if (office_find_iterator != this->office_title_names.end()) {
		const auto find_iterator = office_find_iterator->second.find(tier);
		if (find_iterator != office_find_iterator->second.end()) {
			auto sub_find_iterator = find_iterator->second.find(gender);
			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}

			sub_find_iterator = find_iterator->second.find(gender::none);
			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}
	}

	return this->get_group()->get_office_title_name(office, tier, gender);
}

QString government_type::get_modifier_string() const
{
	if (this->get_modifier() == nullptr) {
		return QString();
	}

	return QString::fromStdString(this->get_modifier()->get_string(nullptr));
}

}
