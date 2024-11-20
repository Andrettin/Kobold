#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("item/item_slot.h")
Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class icon;
class item_slot;

class item final : public named_data_entry, public data_type<item>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::item_slot* slot MEMBER slot READ get_slot NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "item";
	static constexpr const char property_class_identifier[] = "kobold::item*";
	static constexpr const char database_folder[] = "items";

	explicit item(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const item_slot *get_slot() const
	{
		return this->slot;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

signals:
	void changed();

private:
	const item_slot *slot = nullptr;
	const kobold::icon *icon = nullptr;
};

}
