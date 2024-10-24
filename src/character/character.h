#pragma once

#include "character/character_base.h"
#include "database/data_type.h"
#include "util/fractional_int.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("character/character_class.h")
Q_MOC_INCLUDE("character/character_game_data.h")
Q_MOC_INCLUDE("character/dynasty.h")
Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("country/religion.h")
Q_MOC_INCLUDE("map/site.h")
Q_MOC_INCLUDE("population/phenotype.h")
Q_MOC_INCLUDE("ui/portrait.h")

namespace archimedes {
	class calendar;
	enum class gender;
}

namespace kobold {

class character_class;
class character_game_data;
class character_history;
class civilian_unit_class;
class civilian_unit_type;
class country;
class culture;
class dynasty;
class phenotype;
class portrait;
class religion;
class site;
class trait;
enum class character_class_type;
enum class character_role;
enum class military_unit_category;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class character final : public character_base, public data_type<character>
{
	Q_OBJECT

	Q_PROPERTY(kobold::dynasty* dynasty MEMBER dynasty NOTIFY changed)
	Q_PROPERTY(kobold::character_role role MEMBER role READ get_role NOTIFY changed)
	Q_PROPERTY(int level MEMBER level READ get_level NOTIFY changed)
	Q_PROPERTY(std::string rank MEMBER rank NOTIFY changed)
	Q_PROPERTY(kobold::culture* culture MEMBER culture NOTIFY changed)
	Q_PROPERTY(kobold::religion* religion MEMBER religion NOTIFY changed)
	Q_PROPERTY(kobold::phenotype* phenotype MEMBER phenotype NOTIFY changed)
	Q_PROPERTY(kobold::portrait* portrait MEMBER portrait NOTIFY changed)
	Q_PROPERTY(const kobold::site* home_settlement MEMBER home_settlement NOTIFY changed)
	Q_PROPERTY(const kobold::site* home_site MEMBER home_site NOTIFY changed)
	Q_PROPERTY(const kobold::character* father READ get_father WRITE set_father NOTIFY changed)
	Q_PROPERTY(const kobold::character* mother READ get_mother WRITE set_mother NOTIFY changed)
	Q_PROPERTY(kobold::character_game_data* game_data READ get_game_data NOTIFY game_data_changed)

public:
	static constexpr const char property_class_identifier[] = "kobold::character*";
	static constexpr bool history_enabled = true;

	static const std::set<std::string> database_dependencies;

	explicit character(const std::string &identifier);
	~character();

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;
	virtual data_entry_history *get_history_base() override;

	character_history *get_history() const
	{
		return this->history.get();
	}

	virtual void reset_history() override;

	void reset_game_data();

	character_game_data *get_game_data() const
	{
		return this->game_data.get();
	}

	const dynasty *get_dynasty() const
	{
		return this->dynasty;
	}

	character_role get_role() const
	{
		return this->role;
	}

	const std::map<character_class_type, const character_class *> &get_character_classes() const
	{
		return this->character_classes;
	}

	const character_class *get_character_class(const character_class_type type) const
	{
		const auto find_iterator = this->character_classes.find(type);

		if (find_iterator != this->character_classes.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	int get_level() const
	{
		return this->level;
	}

	const kobold::culture *get_culture() const
	{
		return this->culture;
	}

	const kobold::religion *get_religion() const
	{
		return this->religion;
	}

	const kobold::phenotype *get_phenotype() const
	{
		return this->phenotype;
	}

	const kobold::portrait *get_portrait() const
	{
		return this->portrait;
	}

	const site *get_home_settlement() const
	{
		return this->home_settlement;
	}

	const character *get_father() const
	{
		return static_cast<const character *>(character_base::get_father());
	}

	const character *get_mother() const
	{
		return static_cast<const character *>(character_base::get_mother());
	}

	const std::vector<const trait *> &get_traits() const
	{
		return this->traits;
	}

	const and_condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	const std::vector<const country *> &get_rulable_countries() const
	{
		return this->rulable_countries;
	}

	void add_rulable_country(country *country);

signals:
	void changed();
	void game_data_changed() const;

private:
	kobold::dynasty *dynasty = nullptr;
	kobold::character_role role;
	std::map<character_class_type, const character_class *> character_classes;
	int level = 0;
	std::string rank;
	kobold::culture *culture = nullptr;
	kobold::religion *religion = nullptr;
	kobold::phenotype *phenotype = nullptr;
	kobold::portrait *portrait = nullptr;
	const site *home_settlement = nullptr;
	const site *home_site = nullptr;
	std::vector<const country *> rulable_countries;
	std::vector<const trait *> traits;
	std::unique_ptr<const and_condition<country>> conditions;
	qunique_ptr<character_history> history;
	qunique_ptr<character_game_data> game_data;
};

}
