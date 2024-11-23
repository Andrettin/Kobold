#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class character;
class item_type;

template <typename scope_type>
class modifier;

class item_material final : public named_data_entry, public data_type<item_material>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "item_material";
	static constexpr const char property_class_identifier[] = "kobold::item_material*";
	static constexpr const char database_folder[] = "item_materials";

	explicit item_material(const std::string &identifier);
	~item_material();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const data_entry_set<item_type> &get_item_types() const
	{
		return this->item_types;
	}

	const kobold::modifier<const character> *get_modifier() const
	{
		return this->modifier.get();
	}

signals:
	void changed();

private:
	data_entry_set<item_type> item_types;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
};

}
