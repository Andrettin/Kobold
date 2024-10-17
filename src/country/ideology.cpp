#include "kobold.h"

#include "country/ideology.h"

#include "script/condition/and_condition.h"
#include "script/factor.h"
#include "util/assert_util.h"
#include "util/log_util.h"
#include "util/random.h"

namespace kobold {

ideology::ideology(const std::string &identifier) : named_data_entry(identifier)
{
}

ideology::~ideology()
{
}

void ideology::initialize()
{
	if (!this->color.isValid()) {
		log::log_error("Ideology \"" + this->get_identifier() + "\" has no color. A random one will be generated for it.");
		this->color = random::get()->generate_color();
	}

	named_data_entry::initialize();
}

void ideology::check() const
{
	assert_throw(this->get_color().isValid());
}

}
