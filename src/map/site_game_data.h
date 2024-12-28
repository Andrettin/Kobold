#pragma once

#include "economy/commodity_container.h"
#include "infrastructure/building_slot_type_container.h"
#include "script/scripted_modifier_container.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("country/country.h")
Q_MOC_INCLUDE("infrastructure/improvement.h")
Q_MOC_INCLUDE("infrastructure/settlement_type.h")
Q_MOC_INCLUDE("map/province.h")

namespace kobold {

class army;
class building_class;
class building_type;
class country;
class culture;
class improvement;
class phenotype;
class province;
class religion;
class resource;
class scripted_site_modifier;
class settlement_attribute;
class settlement_building_slot;
class settlement_type;
class site;
class tile;
enum class improvement_slot;

class site_game_data final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QPoint tile_pos READ get_tile_pos CONSTANT)
	Q_PROPERTY(const kobold::province* province READ get_province CONSTANT)
	Q_PROPERTY(const kobold::country* owner READ get_owner NOTIFY owner_changed)
	Q_PROPERTY(QString current_cultural_name READ get_current_cultural_name_qstring NOTIFY culture_changed)
	Q_PROPERTY(const kobold::settlement_type* settlement_type READ get_settlement_type NOTIFY settlement_type_changed)
	Q_PROPERTY(const kobold::improvement* improvement READ get_main_improvement NOTIFY improvements_changed)
	Q_PROPERTY(const kobold::improvement* resource_improvement READ get_resource_improvement NOTIFY improvements_changed)
	Q_PROPERTY(QVariantList settlement_attribute_values READ get_settlement_attribute_values_qvariant_list NOTIFY settlement_attribute_values_changed)
	Q_PROPERTY(QVariantList building_slots READ get_building_slots_qvariant_list CONSTANT)
	Q_PROPERTY(QVariantList scripted_modifiers READ get_scripted_modifiers_qvariant_list NOTIFY scripted_modifiers_changed)
	Q_PROPERTY(QVariantList commodity_outputs READ get_commodity_outputs_qvariant_list NOTIFY commodity_outputs_changed)

public:
	explicit site_game_data(const kobold::site *site);

	void do_turn();

	const QPoint &get_tile_pos() const;
	tile *get_tile() const;

	bool is_on_map() const
	{
		return this->get_tile_pos() != QPoint(-1, -1);
	}

	bool is_coastal() const;
	bool is_near_water() const;
	bool has_route() const;

	const province *get_province() const;

	bool is_provincial_capital() const;
	bool is_capital() const;
	bool can_be_capital() const;

	const country *get_owner() const
	{
		return this->owner;
	}

	void set_owner(const country *owner);

	const kobold::culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(const kobold::culture *culture);

	const std::string &get_current_cultural_name() const;

	QString get_current_cultural_name_qstring() const
	{
		return QString::fromStdString(this->get_current_cultural_name());
	}

	const kobold::religion *get_religion() const
	{
		return this->religion;
	}

	void set_religion(const kobold::religion *religion);

	const kobold::settlement_type *get_settlement_type() const
	{
		return this->settlement_type;
	}

	void set_settlement_type(const kobold::settlement_type *settlement_type);
	void check_settlement_type();

	bool is_built() const;

	const resource *get_resource() const;

	bool is_resource_discovered() const
	{
		return this->resource_discovered;
	}

	void set_resource_discovered(const bool discovered)
	{
		this->resource_discovered = discovered;
	}

	const improvement *get_improvement(const improvement_slot slot) const
	{
		const auto find_iterator = this->improvements.find(slot);

		if (find_iterator != this->improvements.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	const improvement *get_main_improvement() const;
	const improvement *get_resource_improvement() const;
	bool has_improvement(const improvement *improvement) const;
	void set_improvement(const improvement_slot slot, const improvement *improvement);

	const data_entry_map<settlement_attribute, int> &get_settlement_attribute_values() const
	{
		return this->settlement_attribute_values;
	}

	QVariantList get_settlement_attribute_values_qvariant_list() const;

	int get_settlement_attribute_value(const settlement_attribute *attribute) const
	{
		const auto find_iterator = this->settlement_attribute_values.find(attribute);
		if (find_iterator != this->settlement_attribute_values.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void change_settlement_attribute_value(const settlement_attribute *attribute, const int change);

	const std::vector<qunique_ptr<settlement_building_slot>> &get_building_slots() const
	{
		return this->building_slots;
	}

	QVariantList get_building_slots_qvariant_list() const;
	void initialize_building_slots();

	settlement_building_slot *get_building_slot(const building_slot_type *slot_type) const
	{
		const auto find_iterator = this->building_slot_map.find(slot_type);

		if (find_iterator != this->building_slot_map.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	const building_type *get_slot_building(const building_slot_type *slot_type) const;
	void set_slot_building(const building_slot_type *slot_type, const building_type *building);
	const building_type *get_building_class_type(const building_class *building_class) const;
	bool has_building(const building_type *building) const;
	bool has_building_or_better(const building_type *building) const;
	bool has_building_class(const building_class *building_class) const;
	bool can_gain_building(const building_type *building) const;
	bool can_gain_building_class(const building_class *building_class) const;
	void add_building(const building_type *building);
	void clear_buildings();
	void check_building_conditions();
	void check_free_buildings();
	bool check_free_building(const building_type *building);
	bool check_free_improvement(const improvement *improvement);

	void on_settlement_built(const int multiplier);
	void on_building_gained(const building_type *building, const int multiplier);
	void on_wonder_gained(const wonder *wonder, const int multiplier);
	void on_improvement_gained(const improvement *improvement, const int multiplier);

	const scripted_site_modifier_map<int> &get_scripted_modifiers() const
	{
		return this->scripted_modifiers;
	}

	QVariantList get_scripted_modifiers_qvariant_list() const;
	bool has_scripted_modifier(const scripted_site_modifier *modifier) const;
	void add_scripted_modifier(const scripted_site_modifier *modifier, const int duration);
	void remove_scripted_modifier(const scripted_site_modifier *modifier);
	void decrement_scripted_modifiers();

	const commodity_map<centesimal_int> &get_base_commodity_outputs() const
	{
		return this->base_commodity_outputs;
	}

	void change_base_commodity_output(const commodity *commodity, const centesimal_int &change);

	const commodity_map<centesimal_int> &get_commodity_outputs() const
	{
		return this->commodity_outputs;
	}

	QVariantList get_commodity_outputs_qvariant_list() const;

	const centesimal_int &get_commodity_output(const commodity *commodity) const
	{
		const auto find_iterator = this->commodity_outputs.find(commodity);
		if (find_iterator != this->commodity_outputs.end()) {
			return find_iterator->second;
		}

		static constexpr centesimal_int zero;
		return zero;
	}

	Q_INVOKABLE int get_commodity_output(kobold::commodity *commodity)
	{
		const kobold::commodity *const_commodity = commodity;
		return this->get_commodity_output(const_commodity).to_int();
	}

	void set_commodity_output(const commodity *commodity, const centesimal_int &output);
	void calculate_commodity_outputs();

	const centesimal_int &get_local_everyday_consumption(const commodity *commodity) const
	{
		const auto find_iterator = this->local_everyday_consumption.find(commodity);

		if (find_iterator != this->local_everyday_consumption.end()) {
			return find_iterator->second;
		}

		static const centesimal_int zero;
		return zero;
	}

	void change_local_everyday_consumption(const commodity *commodity, const centesimal_int &change);

	const centesimal_int &get_local_luxury_consumption(const commodity *commodity) const
	{
		const auto find_iterator = this->local_luxury_consumption.find(commodity);

		if (find_iterator != this->local_luxury_consumption.end()) {
			return find_iterator->second;
		}

		static const centesimal_int zero;
		return zero;
	}

	void change_local_luxury_consumption(const commodity *commodity, const centesimal_int &change);

	const centesimal_int &get_output_modifier() const
	{
		return this->output_modifier;
	}

	void set_output_modifier(const centesimal_int &value)
	{
		if (value == this->get_output_modifier()) {
			return;
		}

		this->output_modifier = value;

		this->calculate_commodity_outputs();
	}

	void change_output_modifier(const centesimal_int &change)
	{
		this->set_output_modifier(this->get_output_modifier() + change);
	}

	const commodity_map<centesimal_int> &get_commodity_output_modifiers() const
	{
		return this->commodity_output_modifiers;
	}

	const centesimal_int &get_commodity_output_modifier(const commodity *commodity) const
	{
		const auto find_iterator = this->commodity_output_modifiers.find(commodity);

		if (find_iterator != this->commodity_output_modifiers.end()) {
			return find_iterator->second;
		}

		static constexpr centesimal_int zero;
		return zero;
	}

	void set_commodity_output_modifier(const commodity *commodity, const centesimal_int &value)
	{
		if (value == this->get_commodity_output_modifier(commodity)) {
			return;
		}

		if (value == 0) {
			this->commodity_output_modifiers.erase(commodity);
		} else {
			this->commodity_output_modifiers[commodity] = value;
		}

		this->calculate_commodity_outputs();
	}

	void change_commodity_output_modifier(const commodity *commodity, const centesimal_int &change)
	{
		this->set_commodity_output_modifier(commodity, this->get_commodity_output_modifier(commodity) + change);
	}

	bool produces_commodity(const commodity *commodity) const
	{
		return this->get_commodity_outputs().contains(commodity);
	}

	bool can_be_visited() const;

signals:
	void owner_changed();
	void culture_changed();
	void religion_changed();
	void settlement_type_changed();
	void improvements_changed();
	void settlement_attribute_values_changed();
	void scripted_modifiers_changed();
	void commodity_outputs_changed();

private:
	const kobold::site *site = nullptr;
	const country *owner = nullptr;
	const kobold::culture *culture = nullptr;
	const kobold::religion *religion = nullptr;
	const kobold::settlement_type *settlement_type = nullptr;
	std::map<improvement_slot, const improvement *> improvements;
	bool resource_discovered = false;
	data_entry_map<settlement_attribute, int> settlement_attribute_values;
	std::vector<qunique_ptr<settlement_building_slot>> building_slots;
	building_slot_type_map<settlement_building_slot *> building_slot_map;
	scripted_site_modifier_map<int> scripted_modifiers;
	commodity_map<centesimal_int> base_commodity_outputs;
	commodity_map<centesimal_int> commodity_outputs;
	commodity_map<centesimal_int> local_everyday_consumption;
	commodity_map<centesimal_int> local_luxury_consumption;
	centesimal_int output_modifier;
	commodity_map<centesimal_int> commodity_output_modifiers;
};

}
