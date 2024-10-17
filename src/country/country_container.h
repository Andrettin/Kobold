#pragma once

namespace kobold {

class country;

struct country_compare final
{
	bool operator()(const country *country, const kobold::country *other_country) const;
};

using country_set = std::set<const country *, country_compare>;

template <typename T>
using country_map = std::map<const country *, T, country_compare>;

}
