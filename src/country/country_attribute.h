#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class country_attribute final : public named_data_entry, public data_type<country_attribute>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "country_attribute";
	static constexpr const char property_class_identifier[] = "kobold::country_attribute*";
	static constexpr const char database_folder[] = "country_attributes";

	explicit country_attribute(const std::string &identifier) : named_data_entry(identifier)
	{
	}

signals:
	void changed();
};

}
