#include "kobold.h"

#include "species/taxonomic_rank.h"

namespace archimedes {

template class enum_converter<kobold::taxonomic_rank>;

template <>
const std::string enum_converter<kobold::taxonomic_rank>::property_class_identifier = "kobold::taxonomic_rank";

template <>
const std::map<std::string, kobold::taxonomic_rank> enum_converter<kobold::taxonomic_rank>::string_to_enum_map = {
	{ "species", kobold::taxonomic_rank::species },
	{ "genus", kobold::taxonomic_rank::genus },
	{ "subtribe", kobold::taxonomic_rank::subtribe },
	{ "tribe", kobold::taxonomic_rank::tribe },
	{ "subfamily", kobold::taxonomic_rank::subfamily },
	{ "family", kobold::taxonomic_rank::family },
	{ "superfamily", kobold::taxonomic_rank::superfamily },
	{ "infraorder", kobold::taxonomic_rank::infraorder },
	{ "suborder", kobold::taxonomic_rank::suborder },
	{ "order", kobold::taxonomic_rank::order },
	{ "infraclass", kobold::taxonomic_rank::infraclass },
	{ "subclass", kobold::taxonomic_rank::subclass },
	{ "class", kobold::taxonomic_rank::class_rank },
	{ "superclass", kobold::taxonomic_rank::superclass },
	{ "infraphylum", kobold::taxonomic_rank::infraphylum },
	{ "subphylum", kobold::taxonomic_rank::subphylum },
	{ "phylum", kobold::taxonomic_rank::phylum },
	{ "superphylum", kobold::taxonomic_rank::superphylum },
	{ "infrakingdom", kobold::taxonomic_rank::infrakingdom },
	{ "subkingdom", kobold::taxonomic_rank::subkingdom },
	{ "kingdom", kobold::taxonomic_rank::kingdom },
	{ "domain", kobold::taxonomic_rank::domain },
	{ "empire", kobold::taxonomic_rank::empire }
};

template <>
const bool enum_converter<kobold::taxonomic_rank>::initialized = enum_converter::initialize();

}
