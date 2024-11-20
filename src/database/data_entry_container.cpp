#include "kobold.h"

#include "database/data_entry_container.h"

#include "character/character_attribute.h"
#include "character/character_class.h"
#include "character/feat.h"
#include "character/saving_throw_type.h"
#include "character/skill.h"
#include "country/country_attribute.h"
#include "country/country_feat.h"
#include "country/country_milestone.h"
#include "country/country_skill.h"
#include "country/office.h"
#include "economy/commodity_unit.h"
#include "item/item_slot.h"
#include "map/settlement_attribute.h"

namespace kobold {

template <typename T>
bool data_entry_compare<T>::operator()(const T *lhs, const T *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

template struct data_entry_compare<character_attribute>;
template struct data_entry_compare<character_class>;
template struct data_entry_compare<commodity_unit>;
template struct data_entry_compare<country_attribute>;
template struct data_entry_compare<country_feat>;
template struct data_entry_compare<country_milestone>;
template struct data_entry_compare<country_skill>;
template struct data_entry_compare<feat>;
template struct data_entry_compare<item_slot>;
template struct data_entry_compare<office>;
template struct data_entry_compare<saving_throw_type>;
template struct data_entry_compare<settlement_attribute>;
template struct data_entry_compare<skill>;

}
