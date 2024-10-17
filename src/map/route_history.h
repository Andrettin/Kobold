#pragma once

#include "database/data_entry_history.h"

Q_MOC_INCLUDE("infrastructure/pathway.h")

namespace kobold {

class pathway;
class route;

class route_history final : public data_entry_history
{
	Q_OBJECT

	Q_PROPERTY(kobold::pathway* pathway MEMBER pathway)

public:
	explicit route_history(const kobold::route *route) : route(route)
	{
	}

	const kobold::pathway *get_pathway() const
	{
		return this->pathway;
	}

private:
	const kobold::route *route = nullptr;
	kobold::pathway *pathway = nullptr;
};

}
