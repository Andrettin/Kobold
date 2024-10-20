#include "kobold.h"

#include "database/data_entry_container.h"

#include "character/character_attribute.h"
#include "character/character_class.h"
#include "character/saving_throw_type.h"

namespace kobold {

template <typename T>
bool data_entry_compare<T>::operator()(const T *lhs, const T *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

template struct data_entry_compare<character_attribute>;
template struct data_entry_compare<character_class>;
template struct data_entry_compare<saving_throw_type>;

}
