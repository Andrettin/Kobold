#pragma once

#include "species/taxon_base.h"

namespace kobold {

class species_base : public taxon_base
{
	Q_OBJECT

protected:
	explicit species_base(const std::string &identifier);
	~species_base();

signals:
	void changed();
};

}
