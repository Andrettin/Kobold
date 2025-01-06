#include "kobold.h"

#include "script/text_processor.h"

#include "character/character.h"
#include "character/character_game_data.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/culture.h"
#include "infrastructure/building_type.h"
#include "infrastructure/settlement_type.h"
#include "map/province.h"
#include "map/province_game_data.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "species/species.h"
#include "util/assert_util.h"
#include "util/gender.h"
#include "util/queue_util.h"
#include "util/string_util.h"

namespace kobold {

std::string text_processor::process_tokens(std::queue<std::string> &&tokens, const bool process_in_game_data, bool &processed) const
{
	Q_UNUSED(process_in_game_data);

	processed = true;

	if (tokens.empty()) {
		return std::string();
	}

	const std::string token = queue::take(tokens);
	std::queue<std::string> subtokens = text_processor::get_subtokens(token);

	const std::string front_subtoken = queue::take(subtokens);

	std::string str;

	if (front_subtoken == "root") {
		str = this->process_scope_variant_tokens(this->context.root_scope, tokens);
	} else if (front_subtoken == "source") {
		str = this->process_scope_variant_tokens(this->context.source_scope, tokens);
	} else if (front_subtoken == "saved_character_scope") {
		const std::string scope_name = queue::take(subtokens);
		str = this->process_character_tokens(this->context.get_saved_scope<const character>(scope_name), tokens);
	} else if (front_subtoken == "saved_site_scope") {
		const std::string scope_name = queue::take(subtokens);
		str = this->process_site_tokens(this->context.get_saved_scope<const site>(scope_name), tokens);
	} else if (front_subtoken == "saved_string") {
		const std::string name = queue::take(subtokens);
		str = this->context.get_saved_string(name);
	} else if (front_subtoken == "saved_building") {
		const std::string name = queue::take(subtokens);
		str = this->process_named_data_entry_tokens(this->context.get_saved_building(name), tokens);
	} else {
		throw std::runtime_error("Failed to process token \"" + token + "\".");
	}

	if (!tokens.empty()) {
		return this->process_string_tokens(std::move(str), std::move(tokens));
	}

	return str;
}

std::string text_processor::process_scope_variant_tokens(const read_only_context::scope_variant_type &scope_variant, std::queue<std::string> &tokens) const
{
	if (std::holds_alternative<const character *>(scope_variant)) {
		return this->process_character_tokens(std::get<const character *>(scope_variant), tokens);
	} else if (std::holds_alternative<const country *>(scope_variant)) {
		return this->process_country_tokens(std::get<const country *>(scope_variant), tokens);
	} else if (std::holds_alternative<const province *>(scope_variant)) {
		return this->process_province_tokens(std::get<const province *>(scope_variant), tokens);
	} else if (std::holds_alternative<const site *>(scope_variant)) {
		return this->process_site_tokens(std::get<const site *>(scope_variant), tokens);
	} else {
		assert_throw(false);
	}

	return std::string();
}

std::string text_processor::process_character_tokens(const character *character, std::queue<std::string> &tokens) const
{
	if (character == nullptr) {
		throw std::runtime_error("No character provided when processing character tokens.");
	}

	if (tokens.empty()) {
		throw std::runtime_error("No tokens provided when processing character tokens.");
	}

	const std::string token = queue::take(tokens);
	std::queue<std::string> subtokens = text_processor::get_subtokens(token);

	const std::string front_subtoken = queue::take(subtokens);

	if (front_subtoken == "full_name") {
		return character->get_full_name();
	} else if (front_subtoken == "personal_pronoun") {
		return get_gender_personal_pronoun(character->get_gender());
	} else if (front_subtoken == "possessive_pronoun") {
		return get_gender_possessive_pronoun(character->get_gender());
	} else if (front_subtoken == "oblique_pronoun") {
		return get_gender_oblique_pronoun(character->get_gender());
	} else if (front_subtoken == "party_species_name") {
		const std::string &species_name = character->get_species()->get_name();

		if (!character->get_game_data()->get_companions().empty()) {
			return string::get_plural_form(species_name);
		}

		return species_name;
	} else if (front_subtoken == "party_species_name_plural") {
		return string::get_plural_form(character->get_species()->get_name());
	} else {
		return this->process_named_data_entry_token(character, front_subtoken);
	}
}

std::string text_processor::process_country_tokens(const country *country, std::queue<std::string> &tokens) const
{
	if (country == nullptr) {
		throw std::runtime_error("No country provided when processing country tokens.");
	}

	if (tokens.empty()) {
		throw std::runtime_error("No tokens provided when processing country tokens.");
	}

	const std::string token = queue::take(tokens);
	std::queue<std::string> subtokens = text_processor::get_subtokens(token);

	const std::string front_subtoken = queue::take(subtokens);

	if (front_subtoken == "title_name") {
		return country->get_game_data()->get_title_name();
	} else {
		return this->process_named_data_entry_token(country, front_subtoken);
	}
}

std::string text_processor::process_culture_tokens(const culture *culture, std::queue<std::string> &tokens) const
{
	if (culture == nullptr) {
		throw std::runtime_error("No culture provided when processing culture tokens.");
	}

	if (tokens.empty()) {
		throw std::runtime_error("No tokens provided when processing culture tokens.");
	}

	const std::string token = queue::take(tokens);
	std::queue<std::string> subtokens = text_processor::get_subtokens(token);

	const std::string front_subtoken = queue::take(subtokens);

	return this->process_named_data_entry_token(culture, front_subtoken);
}

std::string text_processor::process_province_tokens(const province *province, std::queue<std::string> &tokens) const
{
	if (province == nullptr) {
		throw std::runtime_error("No province provided when processing province tokens.");
	}

	if (tokens.empty()) {
		throw std::runtime_error("No tokens provided when processing province tokens.");
	}

	const std::string token = queue::take(tokens);
	std::queue<std::string> subtokens = text_processor::get_subtokens(token);

	const std::string front_subtoken = queue::take(subtokens);

	if (front_subtoken == "name") {
		return province->get_game_data()->get_current_cultural_name();
	} else if (front_subtoken == "country") {
		return this->process_country_tokens(province->get_game_data()->get_owner(), tokens);
	} else {
		return this->process_named_data_entry_token(province, front_subtoken);
	}
}

std::string text_processor::process_settlement_type_tokens(const settlement_type *settlement_type, std::queue<std::string> &tokens) const
{
	if (settlement_type == nullptr) {
		throw std::runtime_error("No settlement type provided when processing settlement type tokens.");
	}

	if (tokens.empty()) {
		throw std::runtime_error("No tokens provided when processing settlement type tokens.");
	}

	const std::string token = queue::take(tokens);
	std::queue<std::string> subtokens = text_processor::get_subtokens(token);

	const std::string front_subtoken = queue::take(subtokens);

	return this->process_named_data_entry_token(settlement_type, front_subtoken);
}

std::string text_processor::process_site_tokens(const site *site, std::queue<std::string> &tokens) const
{
	if (site == nullptr) {
		throw std::runtime_error("No site provided when processing site tokens.");
	}

	if (tokens.empty()) {
		throw std::runtime_error("No tokens provided when processing site tokens.");
	}

	const std::string token = queue::take(tokens);
	std::queue<std::string> subtokens = text_processor::get_subtokens(token);

	const std::string front_subtoken = queue::take(subtokens);

	if (front_subtoken == "name") {
		return site->get_game_data()->get_current_cultural_name();
	} else if (front_subtoken == "country") {
		return this->process_country_tokens(site->get_game_data()->get_owner(), tokens);
	} else if (front_subtoken == "province") {
		return this->process_province_tokens(site->get_game_data()->get_province(), tokens);
	} else if (front_subtoken == "settlement_type") {
		return this->process_settlement_type_tokens(site->get_game_data()->get_settlement_type(), tokens);
	} else {
		return this->process_named_data_entry_token(site, front_subtoken);
	}
}

std::string text_processor::process_species_tokens(const species *species, std::queue<std::string> &tokens) const
{
	if (species == nullptr) {
		throw std::runtime_error("No species provided when processing species tokens.");
	}

	if (tokens.empty()) {
		throw std::runtime_error("No tokens provided when processing species tokens.");
	}

	const std::string token = queue::take(tokens);
	std::queue<std::string> subtokens = text_processor::get_subtokens(token);

	const std::string front_subtoken = queue::take(subtokens);

	return this->process_named_data_entry_token(species, front_subtoken);
}

}
