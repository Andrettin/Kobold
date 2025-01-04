import QtQuick
import QtQuick.Controls

PortraitButton {
	id: character_portrait
	portrait_identifier: portrait ? portrait.identifier : ""
	visible: character !== null
	tooltip: character && character.game_data.country ? format_text(
		character_titled_name
		+ small_text("\n"
			+ "\nClass: " + character.game_data.character_class.name
			+ "\nLevel: " + character.game_data.level
		)
	) : ""
	circle: true
	
	property var character: null
	property var portrait: character && character.game_data.portrait ? character.game_data.portrait : null
	property string character_titled_name: character ? character.game_data.titled_name : ""
	
	onClicked: {
		if (character_dialog.opened && character_dialog.character === character) {
			return
		}
		
		character_dialog.character = character
		character_dialog.open()
	}
}
