#pragma once

#include "database/named_data_entry.h"

namespace kobold {

class divine_domain;

class divine_domain_base : public named_data_entry
{
	Q_OBJECT

public:
	explicit divine_domain_base(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual const divine_domain *get_domain() const = 0;

signals:
	void changed();

private:
};

}
