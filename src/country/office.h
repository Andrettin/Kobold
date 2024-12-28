#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("unit/civilian_unit_class.h")

namespace kobold {

class character_attribute;
class civilian_unit_class;
class country_attribute;

template <typename scope_type>
class and_condition;

class office final : public named_data_entry, public data_type<office>
{
	Q_OBJECT

	Q_PROPERTY(int attribute_modifier_divisor MEMBER attribute_modifier_divisor READ get_attribute_modifier_divisor NOTIFY changed)
	Q_PROPERTY(const kobold::civilian_unit_class* civilian_unit_class MEMBER civilian_unit_class READ get_civilian_unit_class NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "office";
	static constexpr const char property_class_identifier[] = "kobold::office*";
	static constexpr const char database_folder[] = "offices";

	explicit office(const std::string &identifier);
	~office();

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const std::vector<const character_attribute *> &get_character_attributes() const
	{
		return this->character_attributes;
	}

	const std::vector<const country_attribute *> &get_country_attributes() const
	{
		return this->country_attributes;
	}

	int get_attribute_modifier_divisor() const
	{
		return this->attribute_modifier_divisor;
	}

	const kobold::civilian_unit_class *get_civilian_unit_class() const
	{
		return this->civilian_unit_class;
	}

	const and_condition<character> *get_conditions() const
	{
		return this->conditions.get();
	}

signals:
	void changed();

private:
	std::vector<const character_attribute *> character_attributes;
	std::vector<const country_attribute *> country_attributes;
	int attribute_modifier_divisor = 0;
	const kobold::civilian_unit_class *civilian_unit_class = nullptr;
	std::unique_ptr<const and_condition<character>> conditions;
};

}
