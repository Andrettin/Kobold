#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class icon;

class consulate final : public named_data_entry, public data_type<consulate>
{
	Q_OBJECT

	Q_PROPERTY(int level MEMBER level READ get_level NOTIFY changed)
	Q_PROPERTY(kobold::icon* icon MEMBER icon NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "consulate";
	static constexpr const char property_class_identifier[] = "kobold::consulate*";
	static constexpr const char database_folder[] = "consulates";

	explicit consulate(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	int get_level() const
	{
		return this->level;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

signals:
	void changed();

private:
	int level = 0;
	kobold::icon *icon = nullptr;
};

}
