#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class damage_reduction_type final : public named_data_entry, public data_type<damage_reduction_type>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "damage_reduction_type";
	static constexpr const char property_class_identifier[] = "kobold::damage_reduction_type*";
	static constexpr const char database_folder[] = "damage_reduction_types";

	explicit damage_reduction_type(const std::string &identifier) : named_data_entry(identifier)
	{
	}

signals:
	void changed();

private:
};

}
