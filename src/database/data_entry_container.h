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
class commodity_unit;
class country_attribute;
class country_feat;
class country_milestone;
class country_skill;
class feat;
class item_slot;
class office;
class saving_throw_type;
class settlement_attribute;
class skill;

extern template struct data_entry_compare<character_attribute>;
extern template struct data_entry_compare<character_class>;
extern template struct data_entry_compare<commodity_unit>;
extern template struct data_entry_compare<country_attribute>;
extern template struct data_entry_compare<country_feat>;
extern template struct data_entry_compare<country_milestone>;
extern template struct data_entry_compare<country_skill>;
extern template struct data_entry_compare<feat>;
extern template struct data_entry_compare<item_slot>;
extern template struct data_entry_compare<saving_throw_type>;
extern template struct data_entry_compare<settlement_attribute>;
extern template struct data_entry_compare<skill>;

}
