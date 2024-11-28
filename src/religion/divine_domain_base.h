#pragma once

#include "database/named_data_entry.h"

namespace kobold {

class divine_domain_base : public named_data_entry
{
	Q_OBJECT

public:
	explicit divine_domain_base(const std::string &identifier) : named_data_entry(identifier)
	{
	}

signals:
	void changed();

private:
};

}
