#include "kobold.h"

#include "game/event.h"

#include "economy/commodity.h"
#include "engine_interface.h"
#include "game/character_event.h"
#include "game/country_event.h"
#include "game/event_instance.h"
#include "game/event_random_group.h"
#include "game/event_trigger.h"
#include "game/province_event.h"
#include "game/site_event.h"
#include "script/text_processor.h"

namespace kobold {

const std::set<std::string> event::database_dependencies = {
	commodity::class_identifier //so that unit values are initialized
};

const std::set<std::string> character_event::database_dependencies = event::database_dependencies;
const std::set<std::string> country_event::database_dependencies = event::database_dependencies;
const std::set<std::string> province_event::database_dependencies = event::database_dependencies;
const std::set<std::string> site_event::database_dependencies = event::database_dependencies;

event::event(const std::string &identifier) : named_data_entry(identifier), trigger(event_trigger::none)
{
}

void event::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const gsml_operator gsml_operator = property.get_operator();
	const std::string &value = property.get_value();

	if (key == "random_weight") {
		if (gsml_operator != gsml_operator::assignment) {
			throw std::runtime_error("Invalid operator for property \"" + key + "\".");
		}

		this->set_random_weight(std::stoi(value));
	} else {
		data_entry::process_gsml_property(property);
	}
}

void event::initialize()
{
	if (this->get_random_group() != nullptr) {
		if (!this->is_random()) {
			this->set_random(true);
		}
	}
}

void event::create_instance(const context &ctx) const
{
	const text_processor text_processor(ctx);

	const std::string name = text_processor.process_text(this->get_name(), true);
	const std::string description = text_processor.process_text(this->get_description(), true);

	auto event_instance = make_qunique<kobold::event_instance>(this, QString::fromStdString(name), QString::fromStdString(description), ctx);
	engine_interface::get()->add_event_instance(std::move(event_instance));
}

}
