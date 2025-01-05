#pragma once

#include "character/character_base.h"
#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "util/fractional_int.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("character/character_class.h")
Q_MOC_INCLUDE("character/character_game_data.h")
Q_MOC_INCLUDE("character/dynasty.h")
Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("map/site.h")
Q_MOC_INCLUDE("religion/deity.h")
Q_MOC_INCLUDE("religion/religion.h")
Q_MOC_INCLUDE("species/phenotype.h")
Q_MOC_INCLUDE("species/species.h")
Q_MOC_INCLUDE("species/subspecies.h")
Q_MOC_INCLUDE("ui/portrait.h")

namespace archimedes {
	class calendar;
	enum class gender;
}

namespace kobold {

class alignment;
class alignment_axis;
class character_class;
class character_game_data;
class character_history;
class character_reference;
class character_template;
class civilian_unit_class;
class civilian_unit_type;
class country;
class culture;
class deity;
class dynasty;
class feat;
class phenotype;
class portrait;
class religion;
class site;
class species;
class species_base;
class subspecies;
enum class character_class_type;
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
	Q_PROPERTY(kobold::species* species MEMBER species NOTIFY changed)
	Q_PROPERTY(kobold::subspecies* subspecies MEMBER subspecies NOTIFY changed)
	Q_PROPERTY(int level MEMBER level READ get_level NOTIFY changed)
	Q_PROPERTY(std::string rank MEMBER rank NOTIFY changed)
	Q_PROPERTY(kobold::culture* culture MEMBER culture NOTIFY changed)
	Q_PROPERTY(const kobold::religion* religion MEMBER religion NOTIFY changed)
	Q_PROPERTY(const kobold::deity* patron_deity MEMBER patron_deity READ get_patron_deity NOTIFY changed)
	Q_PROPERTY(const kobold::phenotype* phenotype MEMBER phenotype NOTIFY changed)
	Q_PROPERTY(const kobold::deity* deity READ get_deity NOTIFY changed)
	Q_PROPERTY(const kobold::portrait* portrait MEMBER portrait NOTIFY changed)
	Q_PROPERTY(QVariantList alignments READ get_alignments_qvariant_list NOTIFY changed)
	Q_PROPERTY(const kobold::site* home_settlement MEMBER home_settlement NOTIFY changed)
	Q_PROPERTY(const kobold::site* home_site MEMBER home_site NOTIFY changed)
	Q_PROPERTY(kobold::character* father READ get_father WRITE set_father NOTIFY changed)
	Q_PROPERTY(kobold::character* mother READ get_mother WRITE set_mother NOTIFY changed)
	Q_PROPERTY(kobold::character* contemporary_character READ get_contemporary_character WRITE set_contemporary_character NOTIFY changed)
	Q_PROPERTY(kobold::character_game_data* game_data READ get_game_data NOTIFY game_data_changed)

public:
	static constexpr const char property_class_identifier[] = "kobold::character*";
	static constexpr bool history_enabled = true;

	static const std::set<std::string> database_dependencies;

	static void initialize_all();

	static character *generate(const kobold::species *species, const kobold::subspecies *subspecies, const std::map<character_class_type, character_class *> &character_classes, const int level, const kobold::culture *culture, const kobold::religion *religion, const site *home_settlement, const std::vector<const feat *> &feats, const bool temporary = false);
	static character *generate(const character_template *character_template, const kobold::culture *culture, const kobold::religion *religion, const site *home_settlement, const bool temporary = false);
	static std::shared_ptr<character_reference> generate_temporary(const character_template *character_template, const kobold::culture *culture, const kobold::religion *religion, const site *home_settlement);

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

	virtual void initialize_dates() override;
	bool initialize_dates_from_children();
	bool initialize_dates_from_parents();

	const dynasty *get_dynasty() const
	{
		return this->dynasty;
	}

	const kobold::species *get_species() const
	{
		return this->species;
	}

	const kobold::subspecies *get_subspecies() const
	{
		return this->subspecies;
	}

	const species_base *get_species_base() const;
	species_base *get_species_base();

	const std::map<character_class_type, character_class *> &get_character_classes() const
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

	character_class *get_character_class() const
	{
		if (!this->character_classes.empty()) {
			return std::prev(this->character_classes.end())->second;
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

	const kobold::deity *get_patron_deity() const
	{
		return this->patron_deity;
	}

	void generate_patron_deity();

	const kobold::phenotype *get_phenotype() const
	{
		return this->phenotype;
	}

	const kobold::deity *get_deity() const
	{
		return this->deity;
	}

	void set_deity(const kobold::deity *deity)
	{
		this->deity = deity;

		this->patron_deity = deity;
	}

	bool is_deity() const
	{
		return this->get_deity() != nullptr;
	}

	virtual bool is_immortal() const override
	{
		return this->is_deity();
	}

	const kobold::portrait *get_portrait() const
	{
		return this->portrait;
	}

	std::vector<const alignment *> get_alignments() const;
	QVariantList get_alignments_qvariant_list() const;
	bool has_alignment(const alignment *alignment) const;
	void generate_alignments();

	const site *get_home_settlement() const
	{
		return this->home_settlement;
	}

	character *get_father() const
	{
		return static_cast<character *>(character_base::get_father());
	}

	character *get_mother() const
	{
		return static_cast<character *>(character_base::get_mother());
	}

	character *get_contemporary_character() const
	{
		return static_cast<character *>(character_base::get_contemporary_character());
	}

	bool is_alive_on_start_date(const QDate &start_date) const
	{
		if (start_date < this->get_birth_date()) {
			return false;
		}

		if (this->get_death_date().isValid() && start_date >= this->get_death_date()) {
			return false;
		}

		return true;
	}

	const std::vector<const feat *> &get_feats() const
	{
		return this->feats;
	}

	const and_condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	bool is_temporary() const
	{
		return this->temporary;
	}

signals:
	void changed();
	void game_data_changed() const;

private:
	kobold::dynasty *dynasty = nullptr;
	kobold::species *species = nullptr;
	kobold::subspecies *subspecies = nullptr;
	std::map<character_class_type, character_class *> character_classes;
	int level = 0;
	std::string rank;
	kobold::culture *culture = nullptr;
	const kobold::religion *religion = nullptr;
	const kobold::deity *patron_deity = nullptr;
	const kobold::phenotype *phenotype = nullptr;
	const kobold::deity *deity = nullptr; //the deity which the character is (if it is a deity)
	const kobold::portrait *portrait = nullptr;
	data_entry_map<alignment_axis, const alignment *> alignments;
	const site *home_settlement = nullptr;
	const site *home_site = nullptr;
	std::vector<const feat *> feats;
	std::unique_ptr<const and_condition<country>> conditions;
	qunique_ptr<character_history> history;
	qunique_ptr<character_game_data> game_data;
	bool temporary = false;
};

}
