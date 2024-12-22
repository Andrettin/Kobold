#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class icon;

class personality final : public named_data_entry, public data_type<personality>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(std::string opposite_name MEMBER opposite_name NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "personality";
	static constexpr const char property_class_identifier[] = "kobold::personality*";
	static constexpr const char database_folder[] = "personalities";

	explicit personality(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	const std::string &get_opposite_name() const
	{
		return this->opposite_name;
	}

signals:
	void changed();

private:
	const kobold::icon *icon = nullptr;
	std::string opposite_name;
};

}
