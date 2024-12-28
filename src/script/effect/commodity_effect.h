#pragma once

#include "country/country.h"
#include "country/country_game_data.h"
#include "economy/commodity.h"
#include "economy/commodity_unit.h"
#include "script/effect/effect.h"
#include "util/dice.h"
#include "util/number_util.h"
#include "util/random.h"
#include "util/string_util.h"

namespace kobold {

template <typename scope_type>
class commodity_effect final : public effect<scope_type>
{
public:
	explicit commodity_effect(const kobold::commodity *commodity, const gsml_operator effect_operator)
		: effect<scope_type>(effect_operator), commodity(commodity)
	{
	}

	explicit commodity_effect(const kobold::commodity *commodity, const std::string &value, const gsml_operator effect_operator)
		: commodity_effect(commodity, effect_operator)
	{
		auto [value_variant, unit] = commodity->string_to_value_variant_with_unit(value);
		this->quantity_variant = std::move(value_variant);
		this->unit = unit;
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string identifier = "commodity";
		return identifier;
	}

	virtual void process_gsml_property(const gsml_property &property) override
	{
		const std::string &key = property.get_key();
		const std::string &value = property.get_value();

		if (key == "quantity") {
			auto [value_variant, unit] = commodity->string_to_value_variant_with_unit(value);
			this->quantity_variant = std::move(value_variant);
			this->unit = unit;
		} else if (key == "roll_again_threshold") {
			this->roll_again_threshold = std::stoi(value);
		} else {
			effect<scope_type>::process_gsml_property(property);
		}
	}

	virtual void check() const override
	{
		assert_throw(this->commodity != nullptr);
	}

	virtual void do_assignment_effect(scope_type *scope) const override
	{
		const country *country = effect<scope_type>::get_scope_country(scope);

		if (country == nullptr) {
			return;
		}

		country->get_game_data()->set_stored_commodity(this->commodity, this->calculate_quantity());
	}

	virtual void do_addition_effect(scope_type *scope) const override
	{
		const country *country = effect<scope_type>::get_scope_country(scope);

		if (country == nullptr) {
			return;
		}

		int change = this->calculate_quantity();

		const int storage = country->get_game_data()->get_stored_commodity(this->commodity);
		if (change < 0 && std::abs(change) > storage) {
			change = -storage;
		}

		country->get_game_data()->change_stored_commodity(this->commodity, change);
	}

	virtual void do_subtraction_effect(scope_type *scope) const override
	{
		const country *country = effect<scope_type>::get_scope_country(scope);

		if (country == nullptr) {
			return;
		}

		int change = -this->calculate_quantity();

		const int storage = country->get_game_data()->get_stored_commodity(this->commodity);
		if (change < 0 && std::abs(change) > storage) {
			change = -storage;
		}

		country->get_game_data()->change_stored_commodity(this->commodity, change);
	}

	int calculate_quantity() const
	{
		int quantity = 0;

		if (std::holds_alternative<dice>(this->quantity_variant)) {
			const dice dice = std::get<archimedes::dice>(this->quantity_variant);

			int roll_result = random::get()->roll_dice(dice);
			quantity = roll_result;

			if (this->roll_again_threshold > 0) {
				while (roll_result >= this->roll_again_threshold) {
					roll_result = random::get()->roll_dice(dice);
					quantity += roll_result;
				}
			}
		} else {
			quantity = std::get<int>(this->quantity_variant);
		}

		if (this->unit != nullptr) {
			quantity *= this->commodity->get_unit_value(this->unit);
		}

		return quantity;
	}

	virtual std::string get_assignment_string() const override
	{
		return std::format("Set {} to {}{}", string::highlight(this->commodity->get_name()), this->get_quantity_string(), this->get_suffix_string());
	}

	virtual std::string get_addition_string() const override
	{
		return std::format("Gain {} {}{}", this->get_quantity_string(), string::highlight(this->commodity->get_name()), this->get_suffix_string());
	}

	virtual std::string get_subtraction_string() const override
	{
		return std::format("Lose {} {}{}", this->get_quantity_string(), string::highlight(this->commodity->get_name()), this->get_suffix_string());
	}

	std::string get_quantity_string() const
	{
		std::string str;

		if (std::holds_alternative<dice>(this->quantity_variant)) {
			const dice dice = std::get<archimedes::dice>(this->quantity_variant);
			str = dice.to_string();
		} else {
			str = number::to_formatted_string(std::get<int>(this->quantity_variant));
		}

		if (this->unit != nullptr) {
			str += " " + this->unit->get_suffix();
		}

		return str;
	}

	std::string get_suffix_string() const
	{
		std::string str;

		if (std::holds_alternative<dice>(this->quantity_variant) && this->roll_again_threshold > 0) {
			const dice dice = std::get<archimedes::dice>(this->quantity_variant);
			str += std::format(" (roll again on {}{})", this->roll_again_threshold, this->roll_again_threshold < dice.get_maximum_result() ? "+" : "");
		}

		return str;
	}

private:
	const kobold::commodity *commodity = nullptr;
	std::variant<int, dice> quantity_variant;
	const commodity_unit *unit = nullptr;
	int roll_again_threshold = 0;
};

}
