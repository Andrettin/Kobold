#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class country_attribute;
class icon;

class country_skill final : public named_data_entry, public data_type<country_skill>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::country_attribute* attribute MEMBER attribute READ get_attribute NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "country_skill";
	static constexpr const char property_class_identifier[] = "kobold::country_skill*";
	static constexpr const char database_folder[] = "country_skills";

	explicit country_skill(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const country_attribute *get_attribute() const
	{
		return this->attribute;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

signals:
	void changed();

private:
	const country_attribute *attribute = nullptr;
	const kobold::icon *icon = nullptr;
};

}
