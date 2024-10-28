#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class character_attribute;

class skill final : public named_data_entry, public data_type<skill>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::character_attribute* attribute MEMBER attribute READ get_attribute NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "skill";
	static constexpr const char property_class_identifier[] = "kobold::skill*";
	static constexpr const char database_folder[] = "skills";

	explicit skill(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const character_attribute *get_attribute() const
	{
		return this->attribute;
	}

signals:
	void changed();

private:
	const character_attribute *attribute = nullptr;
};

}
