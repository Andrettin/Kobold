#include "kobold.h"

#include "country/subject_type.h"

namespace kobold {

subject_type::subject_type(const std::string &identifier) : named_data_entry(identifier)
{
}

subject_type::~subject_type()
{
}

}
