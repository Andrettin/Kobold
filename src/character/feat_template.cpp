#include "kobold.h"

#include "character/feat_template.h"

#include "character/feat.h"
#include "character/feat_template_type.h"
#include "character/skill.h"
#include "country/country_feat.h"
#include "country/country_skill.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace kobold {

const std::set<std::string> feat_template::database_dependencies = {
	skill::class_identifier
};

feat_template::feat_template(const std::string &identifier)
	: data_entry(identifier)
{
}

feat_template::~feat_template()
{
}

void feat_template::process_gsml_property(const gsml_property &property)
{
	if (property.get_key() == "template_type") {
		this->template_type = enum_converter<feat_template_type>::to_enum(property.get_value());

		switch (this->get_template_type()) {
			case feat_template_type::skill:
				for (const skill *skill : skill::get_all()) {
					feat *feat = feat::add(this->process_string_for_skill(this->get_identifier(), skill), this->get_module());
					this->feats.push_back(feat);
				}
				break;
			case feat_template_type::country_skill:
				for (const country_skill *skill : country_skill::get_all()) {
					country_feat *feat = country_feat::add(this->process_string_for_skill(this->get_identifier(), skill), this->get_module());
					this->country_feats.push_back(feat);
				}
				break;
			default:
				assert_throw(false);
		}
	} else {
		switch (this->get_template_type()) {
			case feat_template_type::skill:
				for (size_t i = 0; i < this->feats.size(); ++i) {
					feat *feat = this->feats.at(i);

					const skill *skill = skill::get_all().at(i);
					const gsml_property feat_property = this->process_property_for_skill(property, skill);
					feat->process_gsml_property(feat_property);
				}
				break;
			case feat_template_type::country_skill:
				for (size_t i = 0; i < this->country_feats.size(); ++i) {
					country_feat *feat = this->country_feats.at(i);

					const country_skill *skill = country_skill::get_all().at(i);
					const gsml_property feat_property = this->process_property_for_skill(property, skill);
					feat->process_gsml_property(feat_property);
				}
				break;
			default:
				assert_throw(false);
		}
	}
}

void feat_template::process_gsml_scope(const gsml_data &scope)
{
	switch (this->get_template_type()) {
		case feat_template_type::skill:
			for (size_t i = 0; i < this->feats.size(); ++i) {
				feat *feat = this->feats.at(i);

				const skill *skill = skill::get_all().at(i);
				const gsml_data feat_scope = feat_template::process_scope_for_skill(scope, skill);
				feat->process_gsml_scope(feat_scope);
			}
			break;
		case feat_template_type::country_skill:
			for (size_t i = 0; i < this->country_feats.size(); ++i) {
				country_feat *feat = this->country_feats.at(i);

				const country_skill *skill = country_skill::get_all().at(i);
				const gsml_data feat_scope = feat_template::process_scope_for_skill(scope, skill);
				feat->process_gsml_scope(feat_scope);
			}
			break;
		default:
			assert_throw(false);
	}
}

void feat_template::initialize()
{
	switch (this->get_template_type()) {
		case feat_template_type::skill:
			for (size_t i = 0; i < this->feats.size(); ++i) {
				feat *feat = this->feats.at(i);
				const skill *skill = skill::get_all().at(i);
				feat->set_icon(skill->get_icon());
			}
			break;
		case feat_template_type::country_skill:
			for (size_t i = 0; i < this->country_feats.size(); ++i) {
				country_feat *feat = this->country_feats.at(i);
				const country_skill *skill = country_skill::get_all().at(i);
				feat->set_icon(skill->get_icon());
			}
			break;
		default:
			assert_throw(false);
	}

	data_entry::initialize();
}

void feat_template::check() const
{
	switch (this->get_template_type()) {
		case feat_template_type::skill:
			assert_throw(this->feats.size() == skill::get_all().size());
			break;
		case feat_template_type::country_skill:
			assert_throw(this->country_feats.size() == country_skill::get_all().size());
			break;
		default:
			assert_throw(false);
	}
}

std::string feat_template::process_string_for_skill(const std::string &str, const named_data_entry *skill) const
{
	std::string new_str = str;

	switch (this->get_template_type()) {
		case feat_template_type::skill:
			string::replace(new_str, "[skill]", skill->get_identifier());
			string::replace(new_str, "[skill_name]", skill->get_name());
			break;
		case feat_template_type::country_skill:
			string::replace(new_str, "[country_skill]", skill->get_identifier());
			string::replace(new_str, "[country_skill_name]", skill->get_name());
			break;
		default:
			assert_throw(false);
	}

	return new_str;
}

gsml_property feat_template::process_property_for_skill(const gsml_property &property, const named_data_entry *skill) const
{
	return gsml_property(feat_template::process_string_for_skill(property.get_key(), skill), property.get_operator(), feat_template::process_string_for_skill(property.get_value(), skill));
}

gsml_data feat_template::process_scope_for_skill(const gsml_data &scope, const named_data_entry *skill) const
{
	gsml_data new_scope(this->process_string_for_skill(scope.get_tag(), skill), scope.get_operator());

	for (const std::string &value : scope.get_values()) {
		new_scope.add_value(this->process_string_for_skill(value, skill));
	}

	scope.for_each_element([&](const gsml_property &property) {
		new_scope.add_property(this->process_property_for_skill(property, skill));
	}, [&](const gsml_data &child_scope) {
		new_scope.add_child(this->process_scope_for_skill(child_scope, skill));
	});

	return new_scope;
}

}