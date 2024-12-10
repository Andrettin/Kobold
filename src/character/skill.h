#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("character/skill_category.h")
Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class character_attribute;
class icon;
class skill_category;

class skill final : public named_data_entry, public data_type<skill>
{
	Q_OBJECT

	Q_PROPERTY(kobold::skill_category* category MEMBER category WRITE set_category NOTIFY changed)
	Q_PROPERTY(const kobold::character_attribute* attribute MEMBER attribute NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "skill";
	static constexpr const char property_class_identifier[] = "kobold::skill*";
	static constexpr const char database_folder[] = "skills";

	explicit skill(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const skill_category *get_category() const
	{
		return this->category;
	}

	void set_category(skill_category *category);

	const character_attribute *get_attribute() const;
	const kobold::icon *get_icon() const;

signals:
	void changed();

private:
	skill_category *category = nullptr;
	const character_attribute *attribute = nullptr;
	const kobold::icon *icon = nullptr;
};

}
