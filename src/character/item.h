#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "util/dice.h"

namespace metternich {

class item final : public named_data_entry, public data_type<item>
{
	Q_OBJECT

	Q_PROPERTY(archimedes::dice damage_dice MEMBER damage_dice get_damage_dice NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "item";
	static constexpr const char property_class_identifier[] = "metternich::item*";
	static constexpr const char database_folder[] = "items";

	explicit item(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const dice &get_damage_dice() const
	{
		return this->damage_dice;
	}

signals:
	void changed();

private:
	dice damage_dice;
};

}
