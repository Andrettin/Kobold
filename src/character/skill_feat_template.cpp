#include "kobold.h"

#include "character/skill_feat_template.h"

#include "character/feat.h"
#include "character/skill.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace kobold {

const std::set<std::string> skill_feat_template::database_dependencies = {
	skill::class_identifier
};

skill_feat_template::skill_feat_template(const std::string &identifier)
	: data_entry(identifier)
{
	for (const skill *skill : skill::get_all()) {
		feat *feat = feat::add(skill_feat_template::process_string_for_skill(identifier, skill), this->get_module());
		this->feats.push_back(feat);
	}
}

skill_feat_template::~skill_feat_template()
{
}

void skill_feat_template::process_gsml_property(const gsml_property &property)
{
	for (size_t i = 0; i < this->feats.size(); ++i) {
		feat *feat = this->feats.at(i);

		const skill *skill = skill::get_all().at(i);
		const gsml_property feat_property = skill_feat_template::process_property_for_skill(property, skill);
		feat->process_gsml_property(feat_property);
	}
}

void skill_feat_template::process_gsml_scope(const gsml_data &scope)
{
	for (size_t i = 0; i < this->feats.size(); ++i) {
		feat *feat = this->feats.at(i);

		const skill *skill = skill::get_all().at(i);
		const gsml_data feat_scope = skill_feat_template::process_scope_for_skill(scope, skill);
		feat->process_gsml_scope(feat_scope);
	}
}

void skill_feat_template::initialize()
{
	for (size_t i = 0; i < this->feats.size(); ++i) {
		feat *feat = this->feats.at(i);
		const skill *skill = skill::get_all().at(i);
		feat->set_icon(skill->get_icon());
	}

	data_entry::initialize();
}

void skill_feat_template::check() const
{
	assert_throw(this->feats.size() == skill::get_all().size());
}

std::string skill_feat_template::process_string_for_skill(const std::string &str, const skill *skill)
{
	std::string new_str = str;

	string::replace(new_str, "[skill]", skill->get_identifier());
	string::replace(new_str, "[skill_name]", skill->get_name());

	return new_str;
}

gsml_property skill_feat_template::process_property_for_skill(const gsml_property &property, const skill *skill)
{
	return gsml_property(skill_feat_template::process_string_for_skill(property.get_key(), skill), property.get_operator(), skill_feat_template::process_string_for_skill(property.get_value(), skill));
}

gsml_data skill_feat_template::process_scope_for_skill(const gsml_data &scope, const skill *skill)
{
	gsml_data new_scope(skill_feat_template::process_string_for_skill(scope.get_tag(), skill), scope.get_operator());

	for (const std::string &value : scope.get_values()) {
		new_scope.add_value(skill_feat_template::process_string_for_skill(value, skill));
	}

	scope.for_each_element([&](const gsml_property &property) {
		new_scope.add_property(skill_feat_template::process_property_for_skill(property, skill));
	}, [&](const gsml_data &child_scope) {
		new_scope.add_child(skill_feat_template::process_scope_for_skill(child_scope, skill));
	});

	return new_scope;
}

}
