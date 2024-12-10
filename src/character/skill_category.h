#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class character_attribute;
class icon;
class skill;

class skill_category final : public named_data_entry, public data_type<skill_category>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::character_attribute* attribute MEMBER attribute READ get_attribute NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon READ get_icon NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "skill_category";
	static constexpr const char property_class_identifier[] = "kobold::skill_category*";
	static constexpr const char database_folder[] = "skill_categories";

	explicit skill_category(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const character_attribute *get_attribute() const
	{
		return this->attribute;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	const std::vector<const skill *> &get_skills() const
	{
		return this->skills;
	}

	void add_skill(const skill *skill)
	{
		this->skills.push_back(skill);
	}

signals:
	void changed();

private:
	const character_attribute *attribute = nullptr;
	const kobold::icon *icon = nullptr;
	std::vector<const skill *> skills;
};

}
