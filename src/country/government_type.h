#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("country/government_group.h")
Q_MOC_INCLUDE("ui/icon.h")

namespace archimedes {
	enum class gender;
}

namespace kobold {

class character_class;
class government_group;
class office;
enum class country_tier;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class government_type final : public named_data_entry, public data_type<government_type>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::government_group* group MEMBER group READ get_group NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon READ get_icon NOTIFY changed)
	Q_PROPERTY(QString modifier_string READ get_modifier_string CONSTANT)

public:
	using government_variant = std::variant<const government_type *, const government_group *>;
	using title_name_map = std::map<country_tier, std::string>;
	using office_title_inner_name_map = std::map<country_tier, std::map<gender, std::string>>;
	using office_title_name_map = data_entry_map<office, office_title_inner_name_map>;

	static constexpr const char class_identifier[] = "government_type";
	static constexpr const char property_class_identifier[] = "kobold::government_type*";
	static constexpr const char database_folder[] = "government_types";

	static const std::set<std::string> database_dependencies;

	static void process_title_name_scope(std::map<government_variant, title_name_map> &title_names, const gsml_data &scope);
	static void process_title_name_scope(title_name_map &title_names, const gsml_data &scope);
	static void process_office_title_name_scope(data_entry_map<office, std::map<government_variant, office_title_inner_name_map>> &office_title_names, const gsml_data &scope);
	static void process_office_title_name_scope(office_title_name_map &office_title_names, const gsml_data &scope);
	static void process_office_title_name_scope(std::map<government_variant, office_title_inner_name_map> &office_title_names, const gsml_data &scope);
	static void process_office_title_name_scope(office_title_inner_name_map &office_title_names, const gsml_data &scope);
	static void process_office_title_name_scope(std::map<gender, std::string> &office_title_names, const gsml_data &scope);

	explicit government_type(const std::string &identifier);
	~government_type();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const std::string &get_title_name(const country_tier tier) const;
	const std::string &get_office_title_name(const office *office, const country_tier tier, const gender gender) const;

	const government_group *get_group() const
	{
		return this->group;
	}

	const icon *get_icon() const
	{
		return this->icon;
	}

	const and_condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	const kobold::modifier<const country> *get_modifier() const
	{
		return this->modifier.get();
	}

	const effect_list<const country> *get_effects() const
	{
		return this->effects.get();
	}

	QString get_modifier_string() const;

	const std::vector<const character_class *> &get_ruler_character_classes() const
	{
		return this->ruler_character_classes;
	}

signals:
	void changed();

private:
	const government_group *group = nullptr;
	const icon *icon = nullptr;
	std::unique_ptr<const and_condition<country>> conditions;
	std::unique_ptr<const kobold::modifier<const country>> modifier;
	std::unique_ptr<const effect_list<const country>> effects;
	std::vector<const character_class *> ruler_character_classes;
	title_name_map title_names;
	office_title_name_map office_title_names;
};

}
