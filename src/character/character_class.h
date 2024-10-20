#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class character_class final : public named_data_entry, public data_type<character_class>
{
	Q_OBJECT

	Q_PROPERTY(int base_skill_points_per_level MEMBER base_skill_points_per_level READ get_base_skill_points_per_level NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "character_class";
	static constexpr const char property_class_identifier[] = "kobold::character_class*";
	static constexpr const char database_folder[] = "character_classes";

	explicit character_class(const std::string &identifier);
	~character_class();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	int get_base_skill_points_per_level() const
	{
		return this->base_skill_points_per_level;
	}

signals:
	void changed();

private:
	int base_skill_points_per_level = 0;
};

}
