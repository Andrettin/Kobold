#pragma once

#include "country/consulate_container.h"
#include "country/country_container.h"
#include "country/law_group_container.h"
#include "country/tradition_container.h"
#include "database/data_entry_container.h"
#include "economy/commodity_container.h"
#include "economy/resource_container.h"
#include "infrastructure/building_class_container.h"
#include "infrastructure/building_type_container.h"
#include "infrastructure/building_slot_type_container.h"
#include "infrastructure/improvement_container.h"
#include "map/province_container.h"
#include "map/site_container.h"
#include "map/terrain_type_container.h"
#include "script/opinion_modifier_container.h"
#include "unit/military_unit_type_container.h"
#include "unit/promotion_container.h"
#include "util/fractional_int.h"
#include "util/point_container.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("character/character.h")
Q_MOC_INCLUDE("country/country.h")
Q_MOC_INCLUDE("country/government_type.h")
Q_MOC_INCLUDE("country/journal_entry.h")
Q_MOC_INCLUDE("country/law.h")
Q_MOC_INCLUDE("country/law_group.h")
Q_MOC_INCLUDE("country/religion.h")
Q_MOC_INCLUDE("country/subject_type.h")
Q_MOC_INCLUDE("country/tradition.h")
Q_MOC_INCLUDE("map/site.h")
Q_MOC_INCLUDE("ui/icon.h")
Q_MOC_INCLUDE("unit/military_unit_type.h")

namespace kobold {

class army;
class building_type;
class character;
class civilian_unit;
class consulate;
class country;
class country_attribute;
class country_rank;
class culture;
class event;
class flag;
class government_type;
class journal_entry;
class law;
class military_unit;
class military_unit_type;
class office;
class opinion_modifier;
class phenotype;
class province;
class region;
class religion;
class site;
class subject_type;
class tradition;
class wonder;
enum class country_tier;
enum class diplomacy_state;
enum class diplomatic_map_mode;
enum class event_trigger;
enum class income_transaction_type;
enum class military_unit_category;
enum class military_unit_stat;
struct read_only_context;

class country_game_data final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(kobold::country_tier tier READ get_tier NOTIFY tier_changed)
	Q_PROPERTY(QString name READ get_name_qstring NOTIFY title_name_changed)
	Q_PROPERTY(QString titled_name READ get_titled_name_qstring NOTIFY title_name_changed)
	Q_PROPERTY(QString title_name READ get_title_name_qstring NOTIFY title_name_changed)
	Q_PROPERTY(QString ruler_title_name READ get_ruler_title_name_qstring NOTIFY ruler_title_name_changed)
	Q_PROPERTY(const kobold::religion* religion READ get_religion NOTIFY religion_changed)
	Q_PROPERTY(const kobold::country* overlord READ get_overlord NOTIFY overlord_changed)
	Q_PROPERTY(QString type_name READ get_type_name_qstring NOTIFY type_name_changed)
	Q_PROPERTY(const kobold::subject_type* subject_type READ get_subject_type NOTIFY subject_type_changed)
	Q_PROPERTY(QVariantList attribute_values READ get_attribute_values_qvariant_list NOTIFY attribute_values_changed)
	Q_PROPERTY(QVariantList provinces READ get_provinces_qvariant_list NOTIFY provinces_changed)
	Q_PROPERTY(const kobold::site* capital READ get_capital NOTIFY capital_changed)
	Q_PROPERTY(bool coastal READ is_coastal NOTIFY provinces_changed)
	Q_PROPERTY(bool anarchy READ is_under_anarchy NOTIFY provinces_changed)
	Q_PROPERTY(QRect territory_rect READ get_territory_rect NOTIFY provinces_changed)
	Q_PROPERTY(QPoint territory_rect_center READ get_territory_rect_center NOTIFY provinces_changed)
	Q_PROPERTY(QVariantList contiguous_territory_rects READ get_contiguous_territory_rects_qvariant_list NOTIFY provinces_changed)
	Q_PROPERTY(QRect main_contiguous_territory_rect READ get_main_contiguous_territory_rect NOTIFY provinces_changed)
	Q_PROPERTY(QRect text_rect READ get_text_rect NOTIFY provinces_changed)
	Q_PROPERTY(QVariantList resource_counts READ get_resource_counts_qvariant_list NOTIFY provinces_changed)
	Q_PROPERTY(QVariantList vassal_resource_counts READ get_vassal_resource_counts_qvariant_list NOTIFY diplomacy_states_changed)
	Q_PROPERTY(QVariantList tile_terrain_counts READ get_tile_terrain_counts_qvariant_list NOTIFY provinces_changed)
	Q_PROPERTY(QVariantList vassals READ get_vassals_qvariant_list NOTIFY diplomacy_states_changed)
	Q_PROPERTY(QVariantList subject_type_counts READ get_subject_type_counts_qvariant_list NOTIFY diplomacy_states_changed)
	Q_PROPERTY(QVariantList consulates READ get_consulates_qvariant_list NOTIFY consulates_changed)
	Q_PROPERTY(QRect diplomatic_map_image_rect READ get_diplomatic_map_image_rect NOTIFY diplomatic_map_image_changed)
	Q_PROPERTY(int score READ get_score NOTIFY score_changed)
	Q_PROPERTY(int score_rank READ get_score_rank NOTIFY score_rank_changed)
	Q_PROPERTY(int wealth READ get_wealth NOTIFY stored_commodities_changed)
	Q_PROPERTY(QVariantList available_commodities READ get_available_commodities_qvariant_list NOTIFY available_commodities_changed)
	Q_PROPERTY(QVariantList tradeable_commodities READ get_tradeable_commodities_qvariant_list NOTIFY tradeable_commodities_changed)
	Q_PROPERTY(QVariantList stored_commodities READ get_stored_commodities_qvariant_list NOTIFY stored_commodities_changed)
	Q_PROPERTY(int storage_capacity READ get_storage_capacity NOTIFY storage_capacity_changed)
	Q_PROPERTY(QVariantList commodity_inputs READ get_commodity_inputs_qvariant_list NOTIFY commodity_inputs_changed)
	Q_PROPERTY(QVariantList commodity_outputs READ get_commodity_outputs_qvariant_list NOTIFY commodity_outputs_changed)
	Q_PROPERTY(QVariantList everyday_consumption READ get_everyday_consumption_qvariant_list NOTIFY everyday_consumption_changed)
	Q_PROPERTY(QVariantList luxury_consumption READ get_luxury_consumption_qvariant_list NOTIFY luxury_consumption_changed)
	Q_PROPERTY(QColor diplomatic_map_color READ get_diplomatic_map_color NOTIFY overlord_changed)
	Q_PROPERTY(const kobold::government_type* government_type READ get_government_type NOTIFY government_type_changed)
	Q_PROPERTY(QVariantList laws READ get_laws_qvariant_list NOTIFY laws_changed)
	Q_PROPERTY(QVariantList available_traditions READ get_available_traditions_qvariant_list NOTIFY traditions_changed)
	Q_PROPERTY(int tradition_cost READ get_tradition_cost NOTIFY traditions_changed)
	Q_PROPERTY(const kobold::tradition* next_tradition READ get_next_tradition WRITE set_next_tradition NOTIFY next_tradition_changed)
	Q_PROPERTY(const kobold::tradition* next_belief READ get_next_belief WRITE set_next_belief NOTIFY next_belief_changed)
	Q_PROPERTY(const kobold::character* ruler READ get_ruler NOTIFY ruler_changed)
	Q_PROPERTY(QVariantList office_holders READ get_office_holders_qvariant_list NOTIFY office_holders_changed)
	Q_PROPERTY(QVariantList bids READ get_bids_qvariant_list NOTIFY bids_changed)
	Q_PROPERTY(QVariantList offers READ get_offers_qvariant_list NOTIFY offers_changed)
	Q_PROPERTY(int output_modifier READ get_output_modifier_int NOTIFY output_modifier_changed)
	Q_PROPERTY(int throughput_modifier READ get_throughput_modifier NOTIFY throughput_modifier_changed)
	Q_PROPERTY(QVariantList active_journal_entries READ get_active_journal_entries_qvariant_list NOTIFY journal_entries_changed)
	Q_PROPERTY(QVariantList inactive_journal_entries READ get_inactive_journal_entries_qvariant_list NOTIFY journal_entries_changed)
	Q_PROPERTY(QVariantList finished_journal_entries READ get_finished_journal_entries_qvariant_list NOTIFY journal_entries_changed)

public:
	static constexpr int base_deployment_limit = 10;
	static constexpr int vassal_tax_rate = 50;

	explicit country_game_data(kobold::country *country);
	~country_game_data();

	void do_turn();

	void do_income_phase();
	void collect_taxes();

	void do_production();
	void do_construction();
	void do_trade(country_map<commodity_map<int>> &country_luxury_demands);
	void do_events();
	void do_ai_turn();

	bool is_ai() const;

	country_tier get_tier() const
	{
		return this->tier;
	}

	void set_tier(const country_tier tier);

	const std::string &get_name() const;

	QString get_name_qstring() const
	{
		return QString::fromStdString(this->get_name());
	}

	std::string get_titled_name() const;

	QString get_titled_name_qstring() const
	{
		return QString::fromStdString(this->get_titled_name());
	}

	const std::string &get_title_name() const;

	QString get_title_name_qstring() const
	{
		return QString::fromStdString(this->get_title_name());
	}

	const std::string &get_ruler_title_name() const;

	QString get_ruler_title_name_qstring() const
	{
		return QString::fromStdString(this->get_ruler_title_name());
	}

	const kobold::religion *get_religion() const
	{
		return this->religion;
	}

	void set_religion(const kobold::religion *religion);

	const kobold::country *get_overlord() const
	{
		return this->overlord;
	}

	void set_overlord(const kobold::country *overlord);

	bool is_vassal_of(const kobold::country *country) const;
	bool is_any_vassal_of(const kobold::country *country) const;

	Q_INVOKABLE bool is_any_vassal_of(kobold::country *country)
	{
		const kobold::country *country_const = country;
		return this->is_any_vassal_of(country_const);
	}

	bool is_overlord_of(const kobold::country *country) const;
	bool is_any_overlord_of(const kobold::country *country) const;

	bool is_independent() const
	{
		return this->get_overlord() == nullptr;
	}

	std::string get_type_name() const;

	QString get_type_name_qstring() const
	{
		return QString::fromStdString(this->get_type_name());
	}

	const kobold::subject_type *get_subject_type() const
	{
		return this->subject_type;
	}

	void set_subject_type(const kobold::subject_type *subject_type);

	const data_entry_map<country_attribute, int> &get_attribute_values() const
	{
		return this->attribute_values;
	}

	QVariantList get_attribute_values_qvariant_list() const;

	int get_attribute_value(const country_attribute *attribute) const
	{
		const auto find_iterator = this->attribute_values.find(attribute);
		if (find_iterator != this->attribute_values.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void change_attribute_value(const country_attribute *attribute, const int change);

	int get_consumption() const
	{
		return this->consumption;
	}

	void change_consumption(const int change)
	{
		if (change == 0) {
			return;
		}

		this->consumption += change;
	}

	int get_unrest() const
	{
		return this->unrest;
	}

	void change_unrest(const int change)
	{
		if (change == 0) {
			return;
		}

		this->unrest += change;
	}

	int do_check(const country_attribute *attribute) const;

	const std::vector<const province *> &get_provinces() const
	{
		return this->provinces;
	}

	QVariantList get_provinces_qvariant_list() const;
	void add_province(const province *province);
	void remove_province(const province *province);
	void on_province_gained(const province *province, const int multiplier);

	int get_province_count() const
	{
		return static_cast<int>(this->get_provinces().size());
	}

	void on_site_gained(const site *site, const int multiplier);

	const site *get_capital() const
	{
		return this->capital;
	}

	void set_capital(const site *capital);
	void choose_capital();

	const province *get_capital_province() const;

	int get_settlement_count() const
	{
		return this->settlement_count;
	}

	void change_settlement_count(const int change);

	const std::vector<const province *> &get_border_provinces() const
	{
		return this->border_provinces;
	}

	bool is_alive() const
	{
		return !this->get_provinces().empty();
	}

	bool is_under_anarchy() const
	{
		return this->get_capital() == nullptr;
	}

	bool is_coastal() const
	{
		return this->coastal_province_count > 0;
	}

	const QRect &get_territory_rect() const
	{
		return this->territory_rect;
	}

	void calculate_territory_rect();

	const QPoint &get_territory_rect_center() const
	{
		return this->territory_rect_center;
	}

	void calculate_territory_rect_center();

	const std::vector<QRect> &get_contiguous_territory_rects() const
	{
		return this->contiguous_territory_rects;
	}

	QVariantList get_contiguous_territory_rects_qvariant_list() const;

	const QRect &get_main_contiguous_territory_rect() const
	{
		return this->main_contiguous_territory_rect;
	}

	const QRect &get_text_rect() const
	{
		return this->text_rect;
	}

	void calculate_text_rect();

	const std::vector<QPoint> &get_border_tiles() const
	{
		return this->border_tiles;
	}

	const resource_map<int> &get_resource_counts() const
	{
		return this->resource_counts;
	}

	QVariantList get_resource_counts_qvariant_list() const;

	void change_resource_count(const resource *resource, const int change)
	{
		const int final_count = (this->resource_counts[resource] += change);

		if (final_count == 0) {
			this->resource_counts.erase(resource);
		}
	}

	const resource_map<int> &get_vassal_resource_counts() const
	{
		return this->vassal_resource_counts;
	}

	QVariantList get_vassal_resource_counts_qvariant_list() const;

	void change_vassal_resource_count(const resource *resource, const int change)
	{
		const int final_count = (this->vassal_resource_counts[resource] += change);

		if (final_count == 0) {
			this->vassal_resource_counts.erase(resource);
		}
	}

	const terrain_type_map<int> &get_tile_terrain_counts() const
	{
		return this->tile_terrain_counts;
	}

	QVariantList get_tile_terrain_counts_qvariant_list() const;

	void change_tile_terrain_count(const terrain_type *terrain, const int change)
	{
		const int final_count = (this->tile_terrain_counts[terrain] += change);

		if (final_count == 0) {
			this->tile_terrain_counts.erase(terrain);
		}
	}

	const country_set &get_known_countries() const
	{
		return this->known_countries;
	}

	bool is_country_known(const kobold::country *other_country) const
	{
		return this->get_known_countries().contains(other_country);
	}

	void add_known_country(const kobold::country *other_country);

	void remove_known_country(const kobold::country *other_country)
	{
		this->known_countries.erase(other_country);
	}

	diplomacy_state get_diplomacy_state(const kobold::country *other_country) const;
	void set_diplomacy_state(const kobold::country *other_country, const diplomacy_state state);

	const std::map<diplomacy_state, int> &get_diplomacy_state_counts() const
	{
		return this->diplomacy_state_counts;
	}

	void change_diplomacy_state_count(const diplomacy_state state, const int change);
	Q_INVOKABLE QString get_diplomacy_state_diplomatic_map_suffix(kobold::country *other_country) const;

	bool at_war() const;

	bool can_attack(const kobold::country *other_country) const;

	std::optional<diplomacy_state> get_offered_diplomacy_state(const kobold::country *other_country) const;

	Q_INVOKABLE int get_offered_diplomacy_state_int(kobold::country *other_country) const
	{
		const std::optional<diplomacy_state> state = this->get_offered_diplomacy_state(other_country);

		if (!state.has_value()) {
			return -1;
		}

		return static_cast<int>(state.value());
	}

	void set_offered_diplomacy_state(const kobold::country *other_country, const std::optional<diplomacy_state> &state);

	Q_INVOKABLE void set_offered_diplomacy_state_int(kobold::country *other_country, const int state)
	{
		if (state == -1) {
			this->set_offered_diplomacy_state(other_country, std::nullopt);
		} else {
			this->set_offered_diplomacy_state(other_country, static_cast<diplomacy_state>(state));
		}
	}

	QVariantList get_consulates_qvariant_list() const;

	const consulate *get_consulate(const kobold::country *other_country) const
	{
		const auto find_iterator = this->consulates.find(other_country);

		if (find_iterator != this->consulates.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	void set_consulate(const kobold::country *other_country, const consulate *consulate);

	int get_opinion_of(const kobold::country *other) const;

	int get_base_opinion(const kobold::country *other) const
	{
		const auto find_iterator = this->base_opinions.find(other);
		if (find_iterator != this->base_opinions.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_base_opinion(const kobold::country *other, const int opinion);

	void change_base_opinion(const kobold::country *other, const int change)
	{
		this->set_base_opinion(other, this->get_base_opinion(other) + change);
	}

	const opinion_modifier_map<int> &get_opinion_modifiers_for(const kobold::country *other) const
	{
		static const opinion_modifier_map<int> empty_map;

		const auto find_iterator = this->opinion_modifiers.find(other);
		if (find_iterator != this->opinion_modifiers.end()) {
			return find_iterator->second;
		}

		return empty_map;
	}

	void add_opinion_modifier(const kobold::country *other, const opinion_modifier *modifier, const int duration);
	void remove_opinion_modifier(const kobold::country *other, const opinion_modifier *modifier);

	int get_opinion_weighted_prestige_for(const kobold::country *other) const;

	std::vector<const kobold::country *> get_vassals() const;
	QVariantList get_vassals_qvariant_list() const;
	QVariantList get_subject_type_counts_qvariant_list() const;

	std::vector<const kobold::country *> get_neighbor_countries() const;

	const QColor &get_diplomatic_map_color() const;

	const QImage &get_diplomatic_map_image() const
	{
		return this->diplomatic_map_image;
	}

	QImage prepare_diplomatic_map_image() const;

	[[nodiscard]]
	QCoro::Task<QImage> finalize_diplomatic_map_image(QImage &&image) const;

	[[nodiscard]]
	QCoro::Task<void> create_diplomatic_map_image();

	const QRect &get_diplomatic_map_image_rect() const
	{
		return this->diplomatic_map_image_rect;
	}

	const QImage &get_selected_diplomatic_map_image() const
	{
		return this->selected_diplomatic_map_image;
	}

	const QImage &get_diplomatic_map_mode_image(const diplomatic_map_mode mode) const
	{
		const auto find_iterator = this->diplomatic_map_mode_images.find(mode);
		if (find_iterator != this->diplomatic_map_mode_images.end()) {
			return find_iterator->second;
		}

		throw std::runtime_error("No diplomatic map image found for mode " + std::to_string(static_cast<int>(mode)) + ".");
	}

	[[nodiscard]]
	QCoro::Task<void> create_diplomatic_map_mode_image(const diplomatic_map_mode mode);

	const QImage &get_diplomacy_state_diplomatic_map_image(const diplomacy_state state) const
	{
		const auto find_iterator = this->diplomacy_state_diplomatic_map_images.find(state);
		if (find_iterator != this->diplomacy_state_diplomatic_map_images.end()) {
			return find_iterator->second;
		}

		throw std::runtime_error("No diplomacy state diplomatic map image found for state " + std::to_string(static_cast<int>(state)) + ".");
	}

	[[nodiscard]]
	QCoro::Task<void> create_diplomacy_state_diplomatic_map_image(const diplomacy_state state);

	int get_score() const
	{
		return this->score;
	}

	void change_score(const int change);

	int get_score_rank() const
	{
		return this->score_rank;
	}

	void set_score_rank(const int score_rank)
	{
		if (score_rank == this->get_score_rank()) {
			return;
		}

		this->score_rank = score_rank;
		emit score_rank_changed();
	}

	const country_rank *get_rank() const
	{
		return this->rank;
	}

	void set_rank(const country_rank *rank)
	{
		if (rank == this->get_rank()) {
			return;
		}

		this->rank = rank;
		emit rank_changed();
	}

	int get_total_unit_count() const
	{
		return static_cast<int>(this->civilian_units.size()) + static_cast<int>(this->military_units.size());
	}

	bool has_building(const building_type *building) const;

	int get_settlement_building_count(const building_type *building) const
	{
		const auto find_iterator = this->settlement_building_counts.find(building);

		if (find_iterator != this->settlement_building_counts.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	Q_INVOKABLE int get_settlement_building_count(kobold::building_type *building) const
	{
		const kobold::building_type *const_building = building;
		return this->get_settlement_building_count(const_building);
	}

	void change_settlement_building_count(const building_type *building, const int change);

	void on_wonder_gained(const wonder *wonder, const int multiplier);

	int get_wealth() const;
	void set_wealth(const int wealth);

	void change_wealth(const int change)
	{
		this->set_wealth(this->get_wealth() + change);
	}

	void add_taxable_wealth(const int taxable_wealth, const income_transaction_type tax_income_type);

	const commodity_set &get_available_commodities() const
	{
		return this->available_commodities;
	}

	QVariantList get_available_commodities_qvariant_list() const;

	void add_available_commodity(const commodity *commodity)
	{
		this->available_commodities.insert(commodity);
		emit available_commodities_changed();
	}

	const commodity_set &get_tradeable_commodities() const
	{
		return this->tradeable_commodities;
	}

	QVariantList get_tradeable_commodities_qvariant_list() const;

	void add_tradeable_commodity(const commodity *commodity)
	{
		this->tradeable_commodities.insert(commodity);
		emit tradeable_commodities_changed();
	}

	bool can_trade_commodity(const commodity *commodity) const
	{
		return this->get_tradeable_commodities().contains(commodity);
	}

	const commodity_map<int> &get_stored_commodities() const
	{
		return this->stored_commodities;
	}

	QVariantList get_stored_commodities_qvariant_list() const;

	Q_INVOKABLE int get_stored_commodity(const kobold::commodity *commodity) const
	{
		const auto find_iterator = this->stored_commodities.find(commodity);

		if (find_iterator != this->stored_commodities.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_stored_commodity(const commodity *commodity, const int value);

	void change_stored_commodity(const commodity *commodity, const int value)
	{
		this->set_stored_commodity(commodity, this->get_stored_commodity(commodity) + value);
	}

	int get_storage_capacity() const
	{
		return this->storage_capacity;
	}

	void set_storage_capacity(const int capacity);

	void change_storage_capacity(const int change)
	{
		this->set_storage_capacity(this->get_storage_capacity() + change);
	}

	const commodity_map<int> &get_commodity_inputs() const
	{
		return this->commodity_inputs;
	}

	QVariantList get_commodity_inputs_qvariant_list() const;

	int get_commodity_input(const commodity *commodity) const
	{
		const auto find_iterator = this->commodity_inputs.find(commodity);

		if (find_iterator != this->commodity_inputs.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	Q_INVOKABLE int get_commodity_input(const QString &commodity_identifier) const;
	void change_commodity_input(const commodity *commodity, const int change);

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

	Q_INVOKABLE int get_commodity_output(const QString &commodity_identifier) const;
	void change_commodity_output(const commodity *commodity, const centesimal_int &change);

	int get_net_commodity_output(const commodity *commodity) const
	{
		return this->get_commodity_output(commodity).to_int() - this->get_commodity_input(commodity);
	}

	void calculate_site_commodity_outputs();
	void calculate_site_commodity_output(const commodity *commodity);

	const commodity_map<centesimal_int> &get_everyday_consumption() const
	{
		return this->everyday_consumption;
	}

	QVariantList get_everyday_consumption_qvariant_list() const;

	const centesimal_int &get_everyday_consumption(const commodity *commodity) const
	{
		const auto find_iterator = this->everyday_consumption.find(commodity);

		if (find_iterator != this->everyday_consumption.end()) {
			return find_iterator->second;
		}

		static const centesimal_int zero;
		return zero;
	}

	Q_INVOKABLE int get_everyday_consumption(const QString &commodity_identifier) const;
	void change_everyday_consumption(const commodity *commodity, const centesimal_int &change);

	const commodity_map<centesimal_int> &get_luxury_consumption() const
	{
		return this->luxury_consumption;
	}

	QVariantList get_luxury_consumption_qvariant_list() const;

	const centesimal_int &get_luxury_consumption(const commodity *commodity) const
	{
		const auto find_iterator = this->luxury_consumption.find(commodity);

		if (find_iterator != this->luxury_consumption.end()) {
			return find_iterator->second;
		}

		static const centesimal_int zero;
		return zero;
	}

	Q_INVOKABLE int get_luxury_consumption(const QString &commodity_identifier) const;
	void change_luxury_consumption(const commodity *commodity, const centesimal_int &change);

	const commodity_map<centesimal_int> &get_commodity_demands() const
	{
		return this->commodity_demands;
	}

	const centesimal_int &get_commodity_demand(const commodity *commodity) const
	{
		const auto find_iterator = this->commodity_demands.find(commodity);

		if (find_iterator != this->commodity_demands.end()) {
			return find_iterator->second;
		}

		static const centesimal_int zero;
		return zero;
	}

	void change_commodity_demand(const commodity *commodity, const centesimal_int &change);

	bool produces_commodity(const commodity *commodity) const;

	bool can_declare_war_on(const kobold::country *other_country) const;

	const kobold::government_type *get_government_type() const
	{
		return this->government_type;
	}

	void set_government_type(const kobold::government_type *government_type);
	bool can_have_government_type(const kobold::government_type *government_type) const;
	void check_government_type();

	bool is_tribal() const;

	const law_group_map<const law *> &get_laws() const
	{
		return this->laws;
	}

	QVariantList get_laws_qvariant_list() const;

	Q_INVOKABLE const kobold::law *get_law(const kobold::law_group *law_group) const
	{
		const auto find_iterator = this->get_laws().find(law_group);

		if (find_iterator != this->get_laws().end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	void set_law(const law_group *law_group, const law *law);
	bool has_law(const law *law) const;
	Q_INVOKABLE bool can_have_law(const kobold::law *law) const;
	Q_INVOKABLE bool can_enact_law(const kobold::law *law) const;
	Q_INVOKABLE void enact_law(const kobold::law *law);

	Q_INVOKABLE int get_total_law_cost_modifier() const
	{
		return 100 + this->get_law_cost_modifier();
	}

	void check_laws();

	const tradition_set &get_traditions() const
	{
		return this->traditions;
	}

	Q_INVOKABLE bool has_tradition(const kobold::tradition *tradition) const
	{
		return this->get_traditions().contains(tradition);
	}

	std::vector<const tradition *> get_available_traditions() const;
	QVariantList get_available_traditions_qvariant_list() const;

	Q_INVOKABLE bool can_have_tradition(const kobold::tradition *tradition) const;
	void gain_tradition(const tradition *tradition, const int multiplier);
	void gain_tradition_with_prerequisites(const tradition *tradition);
	void check_traditions();
	void check_beliefs();

	int get_tradition_cost() const
	{
		int cost = 3 * static_cast<int>(this->get_traditions().size()) + 25;
		cost *= 100 + (10 * (this->get_province_count() - 1));
		cost /= 100;

		cost *= 100 + this->get_tradition_cost_modifier();
		cost /= 100;

		return std::max(0, cost);
	}

	const tradition *get_next_tradition() const
	{
		return this->next_tradition;
	}

	void set_next_tradition(const tradition *tradition)
	{
		if (tradition == this->get_next_tradition()) {
			return;
		}

		this->next_tradition = tradition;
		emit next_tradition_changed();
	}

	void choose_next_tradition();

	const tradition *get_next_belief() const
	{
		return this->next_belief;
	}

	void set_next_belief(const tradition *belief)
	{
		if (belief == this->get_next_belief()) {
			return;
		}

		this->next_belief = belief;
		emit next_belief_changed();
	}

	void choose_next_belief();

	const std::vector<const character *> &get_characters() const
	{
		return this->characters;
	}

	void add_character(const character *character)
	{
		this->characters.push_back(character);
	}

	void remove_character(const character *character);
	void check_characters();

	const character *get_ruler() const
	{
		return this->ruler;
	}

	void set_ruler(const character *ruler);
	void check_ruler(const character *previous_ruler);
	void choose_ruler(const character *previous_ruler);
	void generate_ruler();
	void on_ruler_died();

	const data_entry_map<office, const character *> &get_office_holders() const
	{
		return this->office_holders;
	}

	QVariantList get_office_holders_qvariant_list() const;

	const character *get_office_holder(const office *office) const
	{
		const auto find_iterator = this->office_holders.find(office);

		if (find_iterator != this->office_holders.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	void set_office_holder(const office *office, const character *character);
	void apply_office_holder(const office *office, const character *office_holder, const int multiplier);

	const commodity_map<int> &get_bids() const
	{
		return this->bids;
	}

	QVariantList get_bids_qvariant_list() const;

	Q_INVOKABLE int get_bid(const kobold::commodity *commodity) const
	{
		const auto find_iterator = this->bids.find(commodity);

		if (find_iterator != this->bids.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	Q_INVOKABLE void set_bid(const kobold::commodity *commodity, const int value);

	Q_INVOKABLE void change_bid(const kobold::commodity *commodity, const int change)
	{
		this->set_bid(commodity, this->get_bid(commodity) + change);
	}

	const commodity_map<int> &get_offers() const
	{
		return this->offers;
	}

	QVariantList get_offers_qvariant_list() const;

	Q_INVOKABLE int get_offer(const kobold::commodity *commodity) const
	{
		const auto find_iterator = this->offers.find(commodity);

		if (find_iterator != this->offers.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	Q_INVOKABLE void set_offer(const kobold::commodity *commodity, const int value);

	Q_INVOKABLE void change_offer(const kobold::commodity *commodity, const int change)
	{
		this->set_offer(commodity, this->get_offer(commodity) + change);
	}

	void do_sale(const kobold::country *other_country, const commodity *commodity, const int sold_quantity, const bool state_purchase);

	const commodity_map<int> &get_commodity_needs() const
	{
		return this->commodity_needs;
	}

	int get_commodity_need(const kobold::commodity *commodity) const
	{
		const auto find_iterator = this->commodity_needs.find(commodity);

		if (find_iterator != this->commodity_needs.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_commodity_need(const kobold::commodity *commodity, const int value)
	{
		if (value == this->get_commodity_need(commodity)) {
			return;
		}

		if (value == 0) {
			this->commodity_needs.erase(commodity);
		} else {
			this->commodity_needs[commodity] = value;
		}
	}

	void calculate_commodity_needs();

	void assign_trade_orders();

	void add_civilian_unit(qunique_ptr<civilian_unit> &&civilian_unit);
	void remove_civilian_unit(civilian_unit *civilian_unit);

	void add_military_unit(qunique_ptr<military_unit> &&military_unit);
	void remove_military_unit(military_unit *military_unit);

	const std::set<std::string> &get_military_unit_names() const
	{
		return this->military_unit_names;
	}

	void add_army(qunique_ptr<army> &&army);
	void remove_army(army *army);

	const military_unit_type *get_best_military_unit_category_type(const military_unit_category category, const culture *culture) const;
	const military_unit_type *get_best_military_unit_category_type(const military_unit_category category) const;

	int get_deployment_limit() const
	{
		return this->deployment_limit;
	}

	void change_deployment_limit(const int change)
	{
		this->deployment_limit += change;
	}

	int get_entrenchment_bonus_modifier() const
	{
		return this->entrenchment_bonus_modifier;
	}

	void change_entrenchment_bonus_modifier(const int change)
	{
		this->entrenchment_bonus_modifier += change;
	}

	const centesimal_int &get_military_unit_type_stat_modifier(const military_unit_type *type, const military_unit_stat stat) const
	{
		const auto find_iterator = this->military_unit_type_stat_modifiers.find(type);

		if (find_iterator != this->military_unit_type_stat_modifiers.end()) {
			const auto sub_find_iterator = find_iterator->second.find(stat);

			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		static constexpr centesimal_int zero;
		return zero;
	}

	void set_military_unit_type_stat_modifier(const military_unit_type *type, const military_unit_stat stat, const centesimal_int &value);

	void change_military_unit_type_stat_modifier(const military_unit_type *type, const military_unit_stat stat, const centesimal_int &change)
	{
		this->set_military_unit_type_stat_modifier(type, stat, this->get_military_unit_type_stat_modifier(type, stat) + change);
	}

	int get_infantry_cost_modifier() const
	{
		return this->infantry_cost_modifier;
	}

	void change_infantry_cost_modifier(const int change)
	{
		this->infantry_cost_modifier += change;
	}

	int get_cavalry_cost_modifier() const
	{
		return this->cavalry_cost_modifier;
	}

	void change_cavalry_cost_modifier(const int change)
	{
		this->cavalry_cost_modifier += change;
	}

	int get_artillery_cost_modifier() const
	{
		return this->artillery_cost_modifier;
	}

	void change_artillery_cost_modifier(const int change)
	{
		this->artillery_cost_modifier += change;
	}

	int get_warship_cost_modifier() const
	{
		return this->warship_cost_modifier;
	}

	void change_warship_cost_modifier(const int change)
	{
		this->warship_cost_modifier += change;
	}

	int get_unit_upgrade_cost_modifier() const
	{
		return this->unit_upgrade_cost_modifier;
	}

	void change_unit_upgrade_cost_modifier(const int change)
	{
		this->unit_upgrade_cost_modifier += change;
	}

	const centesimal_int &get_output_modifier() const
	{
		return this->output_modifier;
	}

	int get_output_modifier_int() const
	{
		return this->get_output_modifier().to_int();
	}

	void set_output_modifier(const centesimal_int &value);

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

	Q_INVOKABLE int get_commodity_output_modifier(kobold::commodity *commodity) const
	{
		const kobold::commodity *const_commodity = commodity;
		return this->get_commodity_output_modifier(const_commodity).to_int();
	}

	void set_commodity_output_modifier(const commodity *commodity, const centesimal_int &value);

	void change_commodity_output_modifier(const commodity *commodity, const centesimal_int &change)
	{
		this->set_commodity_output_modifier(commodity, this->get_commodity_output_modifier(commodity) + change);
	}

	const commodity_map<centesimal_int> &get_capital_commodity_output_modifiers() const
	{
		return this->capital_commodity_output_modifiers;
	}

	const centesimal_int &get_capital_commodity_output_modifier(const commodity *commodity) const
	{
		const auto find_iterator = this->capital_commodity_output_modifiers.find(commodity);

		if (find_iterator != this->capital_commodity_output_modifiers.end()) {
			return find_iterator->second;
		}

		static constexpr centesimal_int zero;
		return zero;
	}

	void set_capital_commodity_output_modifier(const commodity *commodity, const centesimal_int &value);

	void change_capital_commodity_output_modifier(const commodity *commodity, const centesimal_int &change)
	{
		this->set_capital_commodity_output_modifier(commodity, this->get_capital_commodity_output_modifier(commodity) + change);
	}

	int get_throughput_modifier() const
	{
		return this->throughput_modifier;
	}

	void set_throughput_modifier(const int value);

	void change_throughput_modifier(const int value)
	{
		this->set_throughput_modifier(this->get_throughput_modifier() + value);
	}

	int get_commodity_throughput_modifier(const commodity *commodity) const
	{
		const auto find_iterator = this->commodity_throughput_modifiers.find(commodity);

		if (find_iterator != this->commodity_throughput_modifiers.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	Q_INVOKABLE int get_commodity_throughput_modifier(kobold::commodity *commodity) const
	{
		const kobold::commodity *const_commodity = commodity;
		return this->get_commodity_throughput_modifier(const_commodity);
	}

	void set_commodity_throughput_modifier(const commodity *commodity, const int value);

	void change_commodity_throughput_modifier(const commodity *commodity, const int value)
	{
		this->set_commodity_throughput_modifier(commodity, this->get_commodity_throughput_modifier(commodity) + value);
	}

	int get_improved_resource_commodity_bonus(const commodity *commodity, const resource *resource) const
	{
		const auto find_iterator = this->improved_resource_commodity_bonuses.find(resource);

		if (find_iterator != this->improved_resource_commodity_bonuses.end()) {
			const auto sub_find_iterator = find_iterator->second.find(commodity);

			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		return 0;
	}

	void change_improved_resource_commodity_bonus(const resource *resource, const commodity *commodity, const int change);

	const improvement_map<commodity_map<centesimal_int>> &get_improvement_commodity_bonuses() const
	{
		return this->improvement_commodity_bonuses;
	}

	const commodity_map<centesimal_int> &get_improvement_commodity_bonuses(const improvement *improvement) const
	{
		const auto find_iterator = this->improvement_commodity_bonuses.find(improvement);

		if (find_iterator != this->improvement_commodity_bonuses.end()) {
			return find_iterator->second;
		}

		static const commodity_map<centesimal_int> empty_map;
		return empty_map;
	}

	const centesimal_int &get_improvement_commodity_bonus(const commodity *commodity, const improvement *improvement) const
	{
		const auto find_iterator = this->improvement_commodity_bonuses.find(improvement);

		if (find_iterator != this->improvement_commodity_bonuses.end()) {
			const auto sub_find_iterator = find_iterator->second.find(commodity);

			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		static constexpr centesimal_int zero;
		return zero;
	}

	void change_improvement_commodity_bonus(const improvement *improvement, const commodity *commodity, const centesimal_int &change);

	const building_type_map<commodity_map<int>> &get_building_commodity_bonuses() const
	{
		return this->building_commodity_bonuses;
	}

	const commodity_map<int> &get_building_commodity_bonuses(const building_type *building) const
	{
		const auto find_iterator = this->building_commodity_bonuses.find(building);

		if (find_iterator != this->building_commodity_bonuses.end()) {
			return find_iterator->second;
		}

		static const commodity_map<int> empty_map;
		return empty_map;
	}

	int get_building_commodity_bonus(const commodity *commodity, const building_type *building) const
	{
		const auto find_iterator = this->building_commodity_bonuses.find(building);

		if (find_iterator != this->building_commodity_bonuses.end()) {
			const auto sub_find_iterator = find_iterator->second.find(commodity);

			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		return 0;
	}

	void change_building_commodity_bonus(const building_type *building, const commodity *commodity, const int change);

	int get_commodity_bonus_for_tile_threshold(const commodity *commodity, const int threshold) const
	{
		const auto find_iterator = this->commodity_bonuses_for_tile_thresholds.find(commodity);

		if (find_iterator != this->commodity_bonuses_for_tile_thresholds.end()) {
			const auto sub_find_iterator = find_iterator->second.find(threshold);

			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		return 0;
	}

	void set_commodity_bonus_for_tile_threshold(const commodity *commodity, const int threshold, const int value);

	void change_commodity_bonus_for_tile_threshold(const commodity *commodity, const int threshold, const int value)
	{
		this->set_commodity_bonus_for_tile_threshold(commodity, threshold, this->get_commodity_bonus_for_tile_threshold(commodity, threshold) + value);
	}

	const commodity_map<centesimal_int> &get_settlement_commodity_bonuses() const
	{
		return this->settlement_commodity_bonuses;
	}

	const centesimal_int &get_settlement_commodity_bonus(const commodity *commodity) const
	{
		const auto find_iterator = this->settlement_commodity_bonuses.find(commodity);

		if (find_iterator != this->settlement_commodity_bonuses.end()) {
			return find_iterator->second;
		}

		static const centesimal_int zero;

		return zero;
	}

	void change_settlement_commodity_bonus(const commodity *commodity, const centesimal_int &change);

	const commodity_map<centesimal_int> &get_capital_commodity_bonuses() const
	{
		return this->capital_commodity_bonuses;
	}

	const centesimal_int &get_capital_commodity_bonus(const commodity *commodity) const
	{
		const auto find_iterator = this->capital_commodity_bonuses.find(commodity);

		if (find_iterator != this->capital_commodity_bonuses.end()) {
			return find_iterator->second;
		}

		static const centesimal_int zero;

		return zero;
	}

	void change_capital_commodity_bonus(const commodity *commodity, const centesimal_int &change);

	int get_law_cost_modifier() const
	{
		return this->law_cost_modifier;
	}

	void change_law_cost_modifier(const int change)
	{
		this->law_cost_modifier += change;
	}

	int get_tradition_cost_modifier() const
	{
		return this->tradition_cost_modifier;
	}

	void change_tradition_cost_modifier(const int change)
	{
		this->tradition_cost_modifier += change;
	}

	int get_building_cost_efficiency_modifier() const
	{
		return this->building_cost_efficiency_modifier;
	}

	void change_building_cost_efficiency_modifier(const int change)
	{
		this->building_cost_efficiency_modifier += change;
	}

	int get_wonder_cost_efficiency_modifier() const
	{
		return this->wonder_cost_efficiency_modifier;
	}

	void change_wonder_cost_efficiency_modifier(const int change)
	{
		this->wonder_cost_efficiency_modifier += change;
	}

	int get_diplomatic_penalty_for_expansion_modifier() const
	{
		return this->diplomatic_penalty_for_expansion_modifier;
	}

	void change_diplomatic_penalty_for_expansion_modifier(const int change)
	{
		this->diplomatic_penalty_for_expansion_modifier += change;
	}

	void decrement_scripted_modifiers();

	Q_INVOKABLE bool is_tile_explored(const QPoint &tile_pos) const;
	bool is_province_discovered(const province *province) const;

	bool is_province_explored(const province *province) const
	{
		//get whether the province has been fully explored
		return this->explored_provinces.contains(province);
	}

	bool is_region_discovered(const region *region) const;

	void explore_tile(const QPoint &tile_pos);
	void explore_province(const province *province);

	bool is_tile_prospected(const QPoint &tile_pos) const
	{
		return this->prospected_tiles.contains(tile_pos);
	}

	void prospect_tile(const QPoint &tile_pos);
	void reset_tile_prospection(const QPoint &tile_pos);

	const std::vector<const journal_entry *> &get_active_journal_entries() const
	{
		return this->active_journal_entries;
	}

	QVariantList get_active_journal_entries_qvariant_list() const;
	void add_active_journal_entry(const journal_entry *journal_entry);
	void remove_active_journal_entry(const journal_entry *journal_entry);

	const std::vector<const journal_entry *> &get_inactive_journal_entries() const
	{
		return this->inactive_journal_entries;
	}

	QVariantList get_inactive_journal_entries_qvariant_list() const;

	const std::vector<const journal_entry *> &get_finished_journal_entries() const
	{
		return this->finished_journal_entries;
	}

	QVariantList get_finished_journal_entries_qvariant_list() const;
	void check_journal_entries(const bool ignore_effects = false, const bool ignore_random_chance = false);
	bool check_potential_journal_entries();
	bool check_inactive_journal_entries();
	bool check_active_journal_entries(const read_only_context &ctx, const bool ignore_effects, const bool ignore_random_chance);

	const building_class_map<int> &get_free_building_class_counts() const
	{
		return this->free_building_class_counts;
	}

	int get_free_building_class_count(const building_class *building_class) const
	{
		const auto find_iterator = this->free_building_class_counts.find(building_class);

		if (find_iterator != this->free_building_class_counts.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_free_building_class_count(const building_class *building_class, const int value);

	void change_free_building_class_count(const building_class *building_class, const int value)
	{
		this->set_free_building_class_count(building_class, this->get_free_building_class_count(building_class) + value);
	}

	const promotion_map<int> &get_free_infantry_promotion_counts() const
	{
		return this->free_infantry_promotion_counts;
	}

	int get_free_infantry_promotion_count(const promotion *promotion) const
	{
		const auto find_iterator = this->free_infantry_promotion_counts.find(promotion);

		if (find_iterator != this->free_infantry_promotion_counts.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_free_infantry_promotion_count(const promotion *promotion, const int value);

	void change_free_infantry_promotion_count(const promotion *promotion, const int value)
	{
		this->set_free_infantry_promotion_count(promotion, this->get_free_infantry_promotion_count(promotion) + value);
	}

	const promotion_map<int> &get_free_cavalry_promotion_counts() const
	{
		return this->free_cavalry_promotion_counts;
	}

	int get_free_cavalry_promotion_count(const promotion *promotion) const
	{
		const auto find_iterator = this->free_cavalry_promotion_counts.find(promotion);

		if (find_iterator != this->free_cavalry_promotion_counts.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_free_cavalry_promotion_count(const promotion *promotion, const int value);

	void change_free_cavalry_promotion_count(const promotion *promotion, const int value)
	{
		this->set_free_cavalry_promotion_count(promotion, this->get_free_cavalry_promotion_count(promotion) + value);
	}

	const promotion_map<int> &get_free_artillery_promotion_counts() const
	{
		return this->free_artillery_promotion_counts;
	}

	int get_free_artillery_promotion_count(const promotion *promotion) const
	{
		const auto find_iterator = this->free_artillery_promotion_counts.find(promotion);

		if (find_iterator != this->free_artillery_promotion_counts.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_free_artillery_promotion_count(const promotion *promotion, const int value);

	void change_free_artillery_promotion_count(const promotion *promotion, const int value)
	{
		this->set_free_artillery_promotion_count(promotion, this->get_free_artillery_promotion_count(promotion) + value);
	}

	const promotion_map<int> &get_free_warship_promotion_counts() const
	{
		return this->free_warship_promotion_counts;
	}

	int get_free_warship_promotion_count(const promotion *promotion) const
	{
		const auto find_iterator = this->free_warship_promotion_counts.find(promotion);

		if (find_iterator != this->free_warship_promotion_counts.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_free_warship_promotion_count(const promotion *promotion, const int value);

	void change_free_warship_promotion_count(const promotion *promotion, const int value)
	{
		this->set_free_warship_promotion_count(promotion, this->get_free_warship_promotion_count(promotion) + value);
	}

	int get_free_consulate_count(const consulate *consulate) const
	{
		const auto find_iterator = this->free_consulate_counts.find(consulate);

		if (find_iterator != this->free_consulate_counts.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_free_consulate_count(const consulate *consulate, const int value);

	void change_free_consulate_count(const consulate *consulate, const int value)
	{
		this->set_free_consulate_count(consulate, this->get_free_consulate_count(consulate) + value);
	}

	bool has_flag(const flag *flag) const
	{
		return this->flags.contains(flag);
	}

	void set_flag(const flag *flag)
	{
		this->flags.insert(flag);
	}

	void clear_flag(const flag *flag)
	{
		this->flags.erase(flag);
	}

	int get_ai_building_desire_modifier(const building_type *building) const
	{
		const auto find_iterator = this->ai_building_desire_modifiers.find(building);

		if (find_iterator != this->ai_building_desire_modifiers.end()) {
			return find_iterator->second;
		}

		return 0;
	}

	void set_ai_building_desire_modifier(const building_type *building, const int value)
	{
		if (value == this->get_ai_building_desire_modifier(building)) {
			return;
		}

		if (value == 0) {
			this->ai_building_desire_modifiers.erase(building);
		} else {
			this->ai_building_desire_modifiers[building] = value;
		}
	}

	void change_ai_building_desire_modifier(const building_type *building, const int value)
	{
		this->set_ai_building_desire_modifier(building, this->get_ai_building_desire_modifier(building) + value);
	}

	int get_ai_settlement_building_desire_modifier(const site *settlement, const building_type *building) const
	{
		const auto find_iterator = this->ai_settlement_building_desire_modifiers.find(settlement);

		if (find_iterator != this->ai_settlement_building_desire_modifiers.end()) {
			const auto sub_find_iterator = find_iterator->second.find(building);

			if (sub_find_iterator != find_iterator->second.end()) {
				return sub_find_iterator->second;
			}
		}

		return 0;
	}

	void set_ai_settlement_building_desire_modifier(const site *settlement, const building_type *building, const int value)
	{
		if (value == this->get_ai_settlement_building_desire_modifier(settlement, building)) {
			return;
		}

		if (value == 0) {
			this->ai_settlement_building_desire_modifiers[settlement].erase(building);

			if (this->ai_settlement_building_desire_modifiers[settlement].empty()) {
				this->ai_settlement_building_desire_modifiers.erase(settlement);
			}
		} else {
			this->ai_settlement_building_desire_modifiers[settlement][building] = value;
		}
	}

	void change_ai_settlement_building_desire_modifier(const site *settlement, const building_type *building, const int value)
	{
		this->set_ai_settlement_building_desire_modifier(settlement, building, this->get_ai_settlement_building_desire_modifier(settlement, building) + value);
	}

signals:
	void tier_changed();
	void title_name_changed();
	void ruler_title_name_changed();
	void religion_changed();
	void overlord_changed();
	void type_name_changed();
	void subject_type_changed();
	void attribute_values_changed();
	void diplomacy_states_changed();
	void offered_diplomacy_states_changed();
	void consulates_changed();
	void provinces_changed();
	void capital_changed();
	void diplomatic_map_image_changed();
	void score_changed();
	void score_rank_changed();
	void rank_changed();
	void settlement_building_counts_changed();
	void available_commodities_changed();
	void tradeable_commodities_changed();
	void stored_commodities_changed();
	void storage_capacity_changed();
	void commodity_inputs_changed();
	void commodity_outputs_changed();
	void everyday_consumption_changed();
	void luxury_consumption_changed();
	void government_type_changed();
	void laws_changed();
	void traditions_changed();
	void next_tradition_changed();
	void tradition_adopted(const tradition *tradition);
	void next_belief_changed();
	void belief_adopted(const tradition *belief);
	void ruler_changed();
	void office_holders_changed();
	void bids_changed();
	void offers_changed();
	void output_modifier_changed();
	void throughput_modifier_changed();
	void prospected_tiles_changed();
	void journal_entries_changed();
	void journal_entry_completed(const journal_entry *journal_entry);

private:
	kobold::country *country = nullptr;
	country_tier tier{};
	const kobold::religion *religion = nullptr;
	const kobold::country *overlord = nullptr;
	const kobold::subject_type *subject_type = nullptr;
	data_entry_map<country_attribute, int> attribute_values;
	int consumption = 0;
	int unrest = 0;
	std::vector<const province *> provinces;
	const site *capital = nullptr;
	int settlement_count = 0; //only includes built settlements
	std::vector<const province *> border_provinces;
	int coastal_province_count = 0;
	QRect territory_rect;
	std::vector<QRect> contiguous_territory_rects;
	QRect main_contiguous_territory_rect;
	QRect text_rect;
	QPoint territory_rect_center = QPoint(-1, -1);
	std::vector<QPoint> border_tiles;
	resource_map<int> resource_counts;
	resource_map<int> vassal_resource_counts;
	terrain_type_map<int> tile_terrain_counts;
	country_set known_countries;
	country_map<diplomacy_state> diplomacy_states;
	std::map<diplomacy_state, int> diplomacy_state_counts;
	country_map<diplomacy_state> offered_diplomacy_states;
	country_map<const consulate *> consulates;
	country_map<int> base_opinions;
	country_map<opinion_modifier_map<int>> opinion_modifiers;
	QImage diplomatic_map_image;
	QImage selected_diplomatic_map_image;
	std::map<diplomatic_map_mode, QImage> diplomatic_map_mode_images;
	std::map<diplomacy_state, QImage> diplomacy_state_diplomatic_map_images;
	QRect diplomatic_map_image_rect;
	int score = 0;
	const country_rank *rank = nullptr;
	int score_rank = 0;
	building_type_map<int> settlement_building_counts;
	commodity_set available_commodities;
	commodity_set tradeable_commodities;
	commodity_map<int> stored_commodities;
	int storage_capacity = 0;
	commodity_map<int> commodity_inputs;
	commodity_map<centesimal_int> commodity_outputs;
	commodity_map<centesimal_int> everyday_consumption;
	commodity_map<centesimal_int> luxury_consumption;
	commodity_map<centesimal_int> commodity_demands;
	const kobold::government_type *government_type = nullptr;
	law_group_map<const law *> laws;
	tradition_set traditions;
	const tradition *next_tradition = nullptr;
	const tradition *next_belief = nullptr;
	std::vector<const character *> characters;
	const character *ruler = nullptr;
	data_entry_map<office, const character *> office_holders;
	commodity_map<int> bids;
	commodity_map<int> offers;
	commodity_map<int> commodity_needs;
	std::vector<qunique_ptr<civilian_unit>> civilian_units;
	std::vector<qunique_ptr<military_unit>> military_units;
	std::set<std::string> military_unit_names;
	std::vector<qunique_ptr<army>> armies;
	int deployment_limit = country_game_data::base_deployment_limit;
	int entrenchment_bonus_modifier = 0;
	military_unit_type_map<std::map<military_unit_stat, centesimal_int>> military_unit_type_stat_modifiers;
	int infantry_cost_modifier = 0;
	int cavalry_cost_modifier = 0;
	int artillery_cost_modifier = 0;
	int warship_cost_modifier = 0;
	int unit_upgrade_cost_modifier = 0;
	centesimal_int output_modifier;
	commodity_map<centesimal_int> commodity_output_modifiers;
	commodity_map<centesimal_int> capital_commodity_output_modifiers;
	int throughput_modifier = 0;
	commodity_map<int> commodity_throughput_modifiers;
	resource_map<commodity_map<int>> improved_resource_commodity_bonuses;
	improvement_map<commodity_map<centesimal_int>> improvement_commodity_bonuses;
	building_type_map<commodity_map<int>> building_commodity_bonuses;
	commodity_map<std::map<int, int>> commodity_bonuses_for_tile_thresholds;
	commodity_map<centesimal_int> settlement_commodity_bonuses;
	commodity_map<centesimal_int> capital_commodity_bonuses;
	int law_cost_modifier = 0;
	int tradition_cost_modifier = 0;
	int building_cost_efficiency_modifier = 0;
	int wonder_cost_efficiency_modifier = 0;
	int diplomatic_penalty_for_expansion_modifier = 0;
	province_set explored_provinces;
	point_set explored_tiles; //used for tiles in partially-explored provinces
	point_set prospected_tiles;
	std::vector<const journal_entry *> active_journal_entries;
	std::vector<const journal_entry *> inactive_journal_entries;
	std::vector<const journal_entry *> finished_journal_entries;
	building_class_map<int> free_building_class_counts;
	promotion_map<int> free_infantry_promotion_counts;
	promotion_map<int> free_cavalry_promotion_counts;
	promotion_map<int> free_artillery_promotion_counts;
	promotion_map<int> free_warship_promotion_counts;
	consulate_map<int> free_consulate_counts;
	std::set<const flag *> flags;
	building_type_map<int> ai_building_desire_modifiers;
	site_map<building_type_map<int>> ai_settlement_building_desire_modifiers;
};

}
