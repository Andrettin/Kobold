import QtQuick
import QtQuick.Controls

PortraitButton {
	id: ruler_portrait
	portrait_identifier: portrait ? portrait.identifier : ""
	visible: ruler !== null
	tooltip: ruler && ruler.game_data.country ? format_text(
		ruler_titled_name
		+ small_text("\n"
			+ "\nClass: " + ruler.game_data.character_class.name
			+ "\nLevel: " + ruler.game_data.level
		)
	) : ""
	circle: true
	
	property var ruler: null
	property var ruler_country: ruler ? ruler : ruler.game_data.country
	property var portrait: ruler && ruler.game_data.portrait ? ruler.game_data.portrait : null
	readonly property string ruler_titled_name: ruler && ruler_country ? (ruler_country.game_data.ruler_title_name + " " + ruler.full_name) : ""
}
