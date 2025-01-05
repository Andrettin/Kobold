#include "kobold.h"

#include "species/species_base.h"

namespace kobold {

species_base::species_base(const std::string &identifier) : taxon_base(identifier)
{
}

species_base::~species_base()
{
}

}
