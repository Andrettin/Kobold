#pragma once

namespace kobold {

template <typename T>
struct data_entry_compare final
{
	bool operator()(const T *lhs, const T *rhs) const;
};

template <typename T>
using data_entry_set = std::set<const T *, data_entry_compare<T>>;

template <typename T, typename U>
using data_entry_map = std::map<const T *, U, data_entry_compare<T>>;

class character_attribute;
class character_class;
class country_attribute;
class saving_throw_type;

extern template struct data_entry_compare<character_attribute>;
extern template struct data_entry_compare<character_class>;
extern template struct data_entry_compare<country_attribute>;
extern template struct data_entry_compare<saving_throw_type>;

}
