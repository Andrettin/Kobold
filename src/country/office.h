#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("character/character_attribute.h")

namespace kobold {

class character_attribute;
class country_attribute;

class office final : public named_data_entry, public data_type<office>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::character_attribute* character_attribute MEMBER character_attribute READ get_character_attribute NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "office";
	static constexpr const char property_class_identifier[] = "kobold::office*";
	static constexpr const char database_folder[] = "offices";

	explicit office(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const kobold::character_attribute *get_character_attribute() const
	{
		return this->character_attribute;
	}

	const std::vector<const country_attribute *> &get_country_attributes() const
	{
		return this->country_attributes;
	}

signals:
	void changed();

private:
	const kobold::character_attribute *character_attribute = nullptr;
	std::vector<const country_attribute *> country_attributes;
};

}
