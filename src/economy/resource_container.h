#pragma once

namespace kobold {

class resource;

struct resource_compare
{
	bool operator()(const resource *resource, const kobold::resource *other_resource) const;
};

using resource_set = std::set<const resource *, resource_compare>;

template <typename T>
using resource_map = std::map<const resource *, T, resource_compare>;

}
