#include "kobold.h"

#include "script/condition/and_condition.h"

#include "database/gsml_operator.h"
#include "script/condition/condition.h"

namespace kobold {

template class and_condition<character>;
template class and_condition<country>;
template class and_condition<military_unit>;
template class and_condition<province>;
template class and_condition<site>;

}
