#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("item/item_class.h")
Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class character;
class icon;
class item_class;
class item_slot;

template <typename scope_type>
class modifier;

class item_type final : public named_data_entry, public data_type<item_type>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::item_class* item_class MEMBER item_class READ get_item_class NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon READ get_icon NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "item_type";
	static constexpr const char property_class_identifier[] = "kobold::item_type*";
	static constexpr const char database_folder[] = "item_types";

	static std::vector<const item_type *> get_weapon_types()
	{
		std::vector<const item_type *> weapon_types;

		for (const item_type *item_type : item_type::get_all()) {
			if (!item_type->is_weapon()) {
				continue;
			}

			
			weapon_types.push_back(item_type);
		}

		return weapon_types;
	}

	explicit item_type(const std::string &identifier);
	~item_type();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const kobold::item_class *get_item_class() const
	{
		return this->item_class;
	}

	const item_slot *get_slot() const;
	bool is_weapon() const;

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	const kobold::modifier<const character> *get_modifier() const
	{
		return this->modifier.get();
	}

signals:
	void changed();

private:
	const kobold::item_class *item_class = nullptr;
	const kobold::icon *icon = nullptr;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
};

}
