#pragma once

#include "database/data_entry_history.h"

namespace kobold {

class site;

class historical_civilian_unit_history final : public data_entry_history
{
	Q_OBJECT

	Q_PROPERTY(kobold::site* site MEMBER site)

public:
	const kobold::site *get_site() const
	{
		return this->site;
	}

	bool is_active() const
	{
		return this->get_site() != nullptr;
	}

private:
	kobold::site *site = nullptr;
};

}
