#pragma once

namespace kobold {

class site;

struct site_compare
{
	bool operator()(const site *site, const kobold::site *other_site) const;
};

using site_set = std::set<const site *, site_compare>;

template <typename T>
using site_map = std::map<const site *, T, site_compare>;

}
