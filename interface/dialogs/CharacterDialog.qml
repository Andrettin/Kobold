import QtQuick
import QtQuick.Controls
import ".."

ModifierDialog {
	id: character_dialog
	title: character ? character.game_data.titled_name : ""
	//date_string: character ? ("Lived: " + date_year_range_string(character.birth_date, character.death_date)) : ""
	//description: character ? character.description : ""
	description: character ? (
		"Class: " + character.game_data.character_class.name
		+ "\nLevel: " + character.game_data.level
		+ "\n\nFeats: " + string_list_to_string(object_list_to_name_list(character.game_data.feats), ", ")
	) : ""
	
	property var character: null
	
	onClosed: {
		character = null
	}
}
