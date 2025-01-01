#pragma once

#include "script/context.h"
#include "util/text_processor_base.h"

namespace kobold {

class building_type;
class character;
class culture;
class settlement_type;
class species;

class text_processor final : public text_processor_base
{
public:
	explicit text_processor(const read_only_context &ctx) : context(ctx)
	{
	}

	virtual std::string process_tokens(std::queue<std::string> &&tokens, const bool process_in_game_data, bool &processed) const override;
	std::string process_scope_variant_tokens(const read_only_context::scope_variant_type &scope_variant, std::queue<std::string> &tokens) const;
	std::string process_character_tokens(const character *character, std::queue<std::string> &tokens) const;
	std::string process_country_tokens(const country *country, std::queue<std::string> &tokens) const;
	std::string process_culture_tokens(const culture *culture, std::queue<std::string> &tokens) const;
	std::string process_province_tokens(const province *province, std::queue<std::string> &tokens) const;
	std::string process_settlement_type_tokens(const settlement_type *settlement_type, std::queue<std::string> &tokens) const;
	std::string process_site_tokens(const site *site, std::queue<std::string> &tokens) const;
	std::string process_species_tokens(const species *species, std::queue<std::string> &tokens) const;

private:
	const read_only_context context;
};

}
