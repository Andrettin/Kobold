import QtQuick
import QtQuick.Controls
import ".."

ModifierDialog {
	id: character_dialog
	title: character ? character.game_data.titled_name : ""
	//date_string: character ? ("Lived: " + date_year_range_string(character.birth_date, character.death_date)) : ""
	//description: character ? character.description : ""
	description: character ? (
		"Species: " + character.species.name
		+ "\nClass: " + character.game_data.character_class.name
		+ "\nLevel: " + character.game_data.level
		+ (character.deity ? ("\nDivine Rank: " + character.deity.divine_rank) : "")
		+ "\nExperience: " + number_string(character.game_data.experience)
		+ "\nChallenge Rating: " + character.game_data.challenge_rating
		+ "\nAge: " + number_string(character.game_data.age)
		+ (character.deity ? ("\nPantheon: " + character.deity.pantheon.name) : (character.patron_deity ? ("\nPatron Deity: " + patron_deity_name) : ("\nReligion: " + character.religion.name)))
		+ "\nAlignment: " + (character.alignments.length == 0 ? "Neutral" : string_list_to_string(object_list_to_name_list(character.alignments), " "))
		+ "\n\nAttributes: " + values_to_string(character.game_data.attribute_values)
		+ "\nHit Points: " + number_string(character.game_data.hit_points)
		+ "\nBase Attack Bonus: " + signed_number_string(character.game_data.base_attack_bonus)
		+ "\nAttack Bonus: " + signed_number_string(character.game_data.attack_bonus) + (character.game_data.weapon_type_attack_bonuses.length > 0 ? (" (" + bonuses_to_string(character.game_data.weapon_type_attack_bonuses, character.game_data.attack_bonus) + ")") : "")
		+ "\n\nSaving Throws: " + bonuses_to_string(character.game_data.saving_throw_bonuses)
		+ (character.game_data.skill_bonuses.length > 0 ? ("\n\nSkills: " + bonuses_to_string(character.game_data.skill_bonuses)) : "")
		+ "\n\nFeats: " + string_list_to_string(object_counts_to_name_list(character.game_data.feats), ", ")
	) : ""
	
	property var character: null
	readonly property string cultural_patron_deity_name: character && character.patron_deity ? character.patron_deity.get_cultural_name_qstring(character.culture) : ""
	readonly property string patron_deity_name: character && character.patron_deity ? (cultural_patron_deity_name + (cultural_patron_deity_name !== character.patron_deity.name ? (" (" + character.patron_deity.name + ")") : "")) : ""
	
	onClosed: {
		character = null
	}
}
