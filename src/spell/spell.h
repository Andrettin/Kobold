#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class character_class;
class icon;
class spell_effect;
enum class spell_target;

class spell final : public named_data_entry, public data_type<spell>
{
	Q_OBJECT

	Q_PROPERTY(kobold::spell_target target MEMBER target READ get_target NOTIFY changed)
	Q_PROPERTY(kobold::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(int mana_cost MEMBER mana_cost READ get_mana_cost NOTIFY changed)
	Q_PROPERTY(int range MEMBER range READ get_range NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "spell";
	static constexpr const char property_class_identifier[] = "kobold::spell*";
	static constexpr const char database_folder[] = "spells";

	explicit spell(const std::string &identifier);
	~spell();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	spell_target get_target() const
	{
		return this->target;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	int get_mana_cost() const
	{
		return this->mana_cost;
	}

	int get_range() const
	{
		return this->range;
	}

	const std::vector<const character_class *> get_character_classes() const
	{
		return this->character_classes;
	}

	bool is_available_for_character_class(const character_class *character_class) const;

signals:
	void changed();

private:
	spell_target target;
	kobold::icon *icon = nullptr;
	int mana_cost = 0;
	int range = 0;
	std::vector<const character_class *> character_classes;
	std::vector<qunique_ptr<spell_effect>> effects;
};

}
