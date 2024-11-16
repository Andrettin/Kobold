#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class settlement_attribute final : public named_data_entry, public data_type<settlement_attribute>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "settlement_attribute";
	static constexpr const char property_class_identifier[] = "kobold::settlement_attribute*";
	static constexpr const char database_folder[] = "settlement_attributes";

	explicit settlement_attribute(const std::string &identifier) : named_data_entry(identifier)
	{
	}

signals:
	void changed();
};

}
