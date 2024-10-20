#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class character;
class character_attribute;
class country;
class icon;
class military_unit;
class province;
enum class trait_type;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class modifier;

class trait final : public named_data_entry, public data_type<trait>
{
	Q_OBJECT

	Q_PROPERTY(kobold::trait_type type MEMBER type NOTIFY changed)
	Q_PROPERTY(kobold::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(int level MEMBER level READ get_level NOTIFY changed)
	Q_PROPERTY(int max_scaling MEMBER max_scaling READ get_max_scaling NOTIFY changed)
	Q_PROPERTY(QString modifier_string READ get_modifier_string CONSTANT)

public:
	static constexpr const char class_identifier[] = "trait";
	static constexpr const char property_class_identifier[] = "kobold::trait*";
	static constexpr const char database_folder[] = "traits";

	explicit trait(const std::string &identifier);
	~trait();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	trait_type get_type() const
	{
		return this->type;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	int get_level() const
	{
		return this->level;
	}

	int get_max_scaling() const
	{
		return this->max_scaling;
	}

	const data_entry_map<character_attribute, int> &get_attribute_bonuses() const
	{
		return this->attribute_bonuses;
	}

	int get_attribute_bonus(const character_attribute *attribute) const
	{
		const auto find_iterator = this->get_attribute_bonuses().find(attribute);
		if (find_iterator != this->get_attribute_bonuses().end()) {
			return find_iterator->second;
		}

		return 0;
	}

	const and_condition<character> *get_conditions() const
	{
		return this->conditions.get();
	}

	const and_condition<character> *get_generation_conditions() const
	{
		return this->generation_conditions.get();
	}

	const kobold::modifier<const character> *get_modifier() const
	{
		return this->modifier.get();
	}

	QString get_modifier_string() const;

signals:
	void changed();

private:
	trait_type type;
	kobold::icon *icon = nullptr;
	int level = 1;
	int max_scaling = std::numeric_limits<int>::max();
	data_entry_map<character_attribute, int> attribute_bonuses;
	std::unique_ptr<const and_condition<character>> conditions;
	std::unique_ptr<const and_condition<character>> generation_conditions;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
};

}
