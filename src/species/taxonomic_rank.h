#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class taxonomic_rank {
	none,
	species,
	genus,
	subtribe,
	tribe,
	subfamily,
	family,
	superfamily,
	infraorder,
	suborder,
	order,
	infraclass,
	subclass,
	class_rank,
	superclass,
	infraphylum,
	subphylum,
	phylum,
	superphylum,
	infrakingdom,
	subkingdom,
	kingdom,
	domain,
	empire
};

}

extern template class archimedes::enum_converter<kobold::taxonomic_rank>;

Q_DECLARE_METATYPE(kobold::taxonomic_rank)
