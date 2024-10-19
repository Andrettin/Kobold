#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "util/fractional_int.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("character/character_game_data.h")
Q_MOC_INCLUDE("character/character_type.h")
Q_MOC_INCLUDE("character/dynasty.h")
Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("country/religion.h")
Q_MOC_INCLUDE("map/site.h")
Q_MOC_INCLUDE("population/phenotype.h")
Q_MOC_INCLUDE("time/calendar.h")
Q_MOC_INCLUDE("ui/portrait.h")

namespace archimedes {
	class calendar;
	enum class gender;
}

namespace kobold {

class character_game_data;
class character_history;
class character_type;
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
enum class character_attribute;
enum class character_role;
enum class military_unit_category;

template <typename scope_type>
class condition;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class character final : public named_data_entry, public data_type<character>
{
	Q_OBJECT

	Q_PROPERTY(kobold::dynasty* dynasty MEMBER dynasty NOTIFY changed)
	Q_PROPERTY(QString surname READ get_surname_qstring NOTIFY changed)
	Q_PROPERTY(std::string nickname MEMBER nickname NOTIFY changed)
	Q_PROPERTY(std::string epithet MEMBER epithet NOTIFY changed)
	Q_PROPERTY(QString full_name READ get_full_name_qstring NOTIFY changed)
	Q_PROPERTY(QString description READ get_description_qstring NOTIFY changed)
	Q_PROPERTY(kobold::character_role role MEMBER role READ get_role NOTIFY changed)
	Q_PROPERTY(const kobold::character_type* character_type MEMBER character_type READ get_character_type NOTIFY changed)
	Q_PROPERTY(kobold::culture* culture MEMBER culture NOTIFY changed)
	Q_PROPERTY(kobold::religion* religion MEMBER religion NOTIFY changed)
	Q_PROPERTY(kobold::phenotype* phenotype MEMBER phenotype NOTIFY changed)
	Q_PROPERTY(kobold::portrait* portrait MEMBER portrait NOTIFY changed)
	Q_PROPERTY(const kobold::site* home_settlement MEMBER home_settlement NOTIFY changed)
	Q_PROPERTY(const kobold::site* home_site MEMBER home_site NOTIFY changed)
	Q_PROPERTY(archimedes::gender gender MEMBER gender NOTIFY changed)
	Q_PROPERTY(kobold::character* father MEMBER father NOTIFY changed)
	Q_PROPERTY(kobold::character* mother MEMBER mother NOTIFY changed)
	Q_PROPERTY(QDate start_date MEMBER start_date READ get_start_date NOTIFY changed)
	Q_PROPERTY(QDate end_date MEMBER end_date READ get_end_date NOTIFY changed)
	Q_PROPERTY(QDate birth_date MEMBER birth_date READ get_birth_date NOTIFY changed)
	Q_PROPERTY(QDate death_date MEMBER death_date READ get_death_date NOTIFY changed)
	Q_PROPERTY(archimedes::calendar* vital_date_calendar MEMBER vital_date_calendar)
	Q_PROPERTY(int skill MEMBER skill READ get_skill NOTIFY changed)
	Q_PROPERTY(archimedes::centesimal_int skill_multiplier READ get_skill_multiplier WRITE set_skill_multiplier NOTIFY changed)
	Q_PROPERTY(QString leader_type_name READ get_leader_type_name_qstring NOTIFY changed)
	Q_PROPERTY(kobold::character_game_data* game_data READ get_game_data NOTIFY game_data_changed)

public:
	static constexpr const char class_identifier[] = "character";
	static constexpr const char property_class_identifier[] = "kobold::character*";
	static constexpr const char database_folder[] = "characters";
	static constexpr bool history_enabled = true;

	static const std::set<std::string> database_dependencies;

	static bool skill_compare(const character *lhs, const character *rhs);

	explicit character(const std::string &identifier);
	~character();

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

	virtual std::string get_scope_name() const override
	{
		return this->get_full_name();
	}

	const dynasty *get_dynasty() const
	{
		return this->dynasty;
	}

	const std::string &get_surname() const
	{
		return this->surname;
	}

	Q_INVOKABLE void set_surname(const std::string &surname)
	{
		this->surname = surname;
	}

	QString get_surname_qstring() const
	{
		return QString::fromStdString(this->get_surname());
	}

	const std::string &get_nickname() const
	{
		return this->nickname;
	}

	const std::string &get_epithet() const
	{
		return this->epithet;
	}

	std::string get_full_name() const;

	QString get_full_name_qstring() const
	{
		return QString::fromStdString(this->get_full_name());
	}

	const std::string &get_description() const
	{
		return this->description;
	}

	Q_INVOKABLE void set_description(const std::string &description)
	{
		this->description = description;
	}

	QString get_description_qstring() const
	{
		return QString::fromStdString(this->get_description());
	}

	character_role get_role() const
	{
		return this->role;
	}

	const kobold::character_type *get_character_type() const
	{
		return this->character_type;
	}

	const military_unit_category get_military_unit_category() const;
	const civilian_unit_class *get_civilian_unit_class() const;
	const civilian_unit_type *get_civilian_unit_type() const;

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

	gender get_gender() const
	{
		return this->gender;
	}

	const character *get_father() const
	{
		return this->father;
	}

	const character *get_mother() const
	{
		return this->mother;
	}

	const QDate &get_start_date() const
	{
		return this->start_date;
	}

	const QDate &get_end_date() const
	{
		return this->end_date;
	}

	const QDate &get_birth_date() const
	{
		return this->birth_date;
	}

	const QDate &get_death_date() const
	{
		return this->death_date;
	}

	character_attribute get_primary_attribute() const;

	int get_skill() const
	{
		return this->skill;
	}

	centesimal_int get_skill_multiplier() const;
	void set_skill_multiplier(const centesimal_int &skill_multiplier);

	const std::vector<const trait *> &get_traits() const
	{
		return this->traits;
	}

	const condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	const std::vector<const country *> &get_rulable_countries() const
	{
		return this->rulable_countries;
	}

	void add_rulable_country(country *country);

	bool is_admiral() const;
	bool is_explorer() const;
	std::string_view get_leader_type_name() const;

	QString get_leader_type_name_qstring() const
	{
		return QString::fromStdString(std::string(this->get_leader_type_name()));
	}

signals:
	void changed();
	void game_data_changed() const;

private:
	kobold::dynasty *dynasty = nullptr;
	std::string surname;
	std::string nickname;
	std::string epithet;
	std::string description;
	kobold::character_role role;
	const kobold::character_type *character_type = nullptr;
	kobold::culture *culture = nullptr;
	kobold::religion *religion = nullptr;
	kobold::phenotype *phenotype = nullptr;
	kobold::portrait *portrait = nullptr;
	const site *home_settlement = nullptr;
	const site *home_site = nullptr;
	archimedes::gender gender;
	character *father = nullptr;
	character *mother = nullptr;
	QDate start_date;
	QDate end_date;
	QDate birth_date;
	QDate death_date;
	calendar *vital_date_calendar = nullptr; //the calendar for the birth, death, start and end dates
	int skill = 0;
	std::vector<const country *> rulable_countries;
	std::vector<const trait *> traits;
	std::unique_ptr<const condition<country>> conditions;
	qunique_ptr<character_history> history;
	qunique_ptr<character_game_data> game_data;
};

}
