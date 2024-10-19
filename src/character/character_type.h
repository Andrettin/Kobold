#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("unit/civilian_unit_class.h")

namespace kobold {

class civilian_unit_class;
class country;
enum class character_attribute;
enum class military_unit_category;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class character_type final : public named_data_entry, public data_type<character_type>
{
	Q_OBJECT

	Q_PROPERTY(kobold::character_attribute attribute MEMBER attribute NOTIFY changed)
	Q_PROPERTY(kobold::military_unit_category military_unit_category MEMBER military_unit_category READ get_military_unit_category NOTIFY changed)
	Q_PROPERTY(const kobold::civilian_unit_class* civilian_unit_class MEMBER civilian_unit_class READ get_civilian_unit_class NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "character_type";
	static constexpr const char property_class_identifier[] = "kobold::character_type*";
	static constexpr const char database_folder[] = "character_types";

	explicit character_type(const std::string &identifier);
	~character_type();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	character_attribute get_attribute() const
	{
		return this->attribute;
	}

	kobold::military_unit_category get_military_unit_category() const
	{
		return this->military_unit_category;
	}

	const kobold::civilian_unit_class *get_civilian_unit_class() const
	{
		return this->civilian_unit_class;
	}

	const modifier<const country> *get_ruler_modifier() const
	{
		return this->ruler_modifier.get();
	}

	const modifier<const country> *get_scaled_ruler_modifier() const
	{
		return this->scaled_ruler_modifier.get();
	}

signals:
	void changed();

private:
	character_attribute attribute;
	kobold::military_unit_category military_unit_category;
	const kobold::civilian_unit_class *civilian_unit_class = nullptr;
	std::unique_ptr<modifier<const country>> ruler_modifier;
	std::unique_ptr<modifier<const country>> scaled_ruler_modifier;
};

}
