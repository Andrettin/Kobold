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

class alignment;
class alignment_axis;
class character_attribute;
class character_class;
class character_template;
class commodity_unit;
class country_attribute;
class country_feat;
class country_milestone;
class country_skill;
class cultural_group;
class damage_reduction_type;
class divine_domain;
class feat;
class feat_type;
class item_class;
class item_slot;
class item_type;
class office;
class saving_throw_type;
class settlement_attribute;
class skill;

extern template struct data_entry_compare<alignment>;
extern template struct data_entry_compare<alignment_axis>;
extern template struct data_entry_compare<character_attribute>;
extern template struct data_entry_compare<character_class>;
extern template struct data_entry_compare<character_template>;
extern template struct data_entry_compare<commodity_unit>;
extern template struct data_entry_compare<country_attribute>;
extern template struct data_entry_compare<country_feat>;
extern template struct data_entry_compare<country_milestone>;
extern template struct data_entry_compare<country_skill>;
extern template struct data_entry_compare<damage_reduction_type>;
extern template struct data_entry_compare<divine_domain>;
extern template struct data_entry_compare<feat>;
extern template struct data_entry_compare<feat_type>;
extern template struct data_entry_compare<item_class>;
extern template struct data_entry_compare<item_slot>;
extern template struct data_entry_compare<item_type>;
extern template struct data_entry_compare<saving_throw_type>;
extern template struct data_entry_compare<settlement_attribute>;
extern template struct data_entry_compare<skill>;

}
