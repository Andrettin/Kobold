#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "economy/commodity_container.h"
#include "util/fractional_int.h"

Q_MOC_INCLUDE("country/cultural_group.h")
Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("technology/technology.h")
Q_MOC_INCLUDE("ui/icon.h")
Q_MOC_INCLUDE("unit/transporter_class.h")

namespace metternich {

class cultural_group;
class culture;
class icon;
class technology;
class transporter_class;
enum class transporter_category;
enum class transporter_stat;

class transporter_type final : public named_data_entry, public data_type<transporter_type>
{
	Q_OBJECT

	Q_PROPERTY(metternich::transporter_class* transporter_class MEMBER transporter_class NOTIFY changed)
	Q_PROPERTY(metternich::culture* culture MEMBER culture NOTIFY changed)
	Q_PROPERTY(metternich::cultural_group* cultural_group MEMBER cultural_group NOTIFY changed)
	Q_PROPERTY(metternich::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(int hit_points MEMBER hit_points READ get_hit_points NOTIFY changed)
	Q_PROPERTY(int cargo MEMBER cargo READ get_cargo NOTIFY changed)
	Q_PROPERTY(metternich::technology* required_technology MEMBER required_technology NOTIFY changed)
	Q_PROPERTY(int wealth_cost MEMBER wealth_cost READ get_wealth_cost NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "transporter_type";
	static constexpr const char property_class_identifier[] = "metternich::transporter_type*";
	static constexpr const char database_folder[] = "transporter_types";

public:
	explicit transporter_type(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	const metternich::transporter_class *get_transporter_class() const
	{
		return this->transporter_class;
	}

	transporter_category get_category() const;
	bool is_ship() const;

	const metternich::culture *get_culture() const
	{
		return this->culture;
	}

	const metternich::cultural_group *get_cultural_group() const
	{
		return this->cultural_group;
	}

	const metternich::icon *get_icon() const
	{
		return this->icon;
	}

	const std::map<transporter_stat, centesimal_int> &get_stats() const
	{
		return this->stats;
	}

	const centesimal_int &get_stat(const transporter_stat stat) const
	{
		const auto find_iterator = this->get_stats().find(stat);
		if (find_iterator != this->get_stats().end()) {
			return find_iterator->second;
		}

		static constexpr centesimal_int zero;
		return zero;
	}

	centesimal_int get_stat_for_country(const transporter_stat stat, const country *country) const;

	int get_hit_points() const
	{
		return this->hit_points;
	}

	int get_cargo() const
	{
		return this->cargo;
	}

	const technology *get_required_technology() const
	{
		return this->required_technology;
	}

	int get_wealth_cost() const
	{
		return this->wealth_cost;
	}

	const commodity_map<int> &get_commodity_costs() const
	{
		return this->commodity_costs;
	}

	const std::vector<const transporter_type *> &get_upgrades() const
	{
		return this->upgrades;
	}

	int get_score() const;

signals:
	void changed();

private:
	metternich::transporter_class *transporter_class = nullptr;
	metternich::culture *culture = nullptr;
	metternich::cultural_group *cultural_group = nullptr;
	metternich::icon *icon = nullptr;
	std::map<transporter_stat, centesimal_int> stats;
	int hit_points = 25;
	int cargo = 0;
	technology *required_technology = nullptr;
	int wealth_cost = 0;
	commodity_map<int> commodity_costs;
	std::vector<const transporter_type *> upgrades;
};

}
