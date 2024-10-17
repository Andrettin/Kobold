#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class subject_type final : public named_data_entry, public data_type<subject_type>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "subject_type";
	static constexpr const char property_class_identifier[] = "kobold::subject_type*";
	static constexpr const char database_folder[] = "subject_types";

	explicit subject_type(const std::string &identifier);
	~subject_type();

signals:
	void changed();
};

}
