#pragma once

#include "database/data_type.h"
#include "religion/divine_domain_base.h"

namespace kobold {

class divine_domain final : public divine_domain_base, public data_type<divine_domain>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "divine_domain";
	static constexpr const char property_class_identifier[] = "kobold::divine_domain*";
	static constexpr const char database_folder[] = "divine_domains";

	explicit divine_domain(const std::string &identifier) : divine_domain_base(identifier)
	{
	}

signals:
	void changed();

private:
};

}
