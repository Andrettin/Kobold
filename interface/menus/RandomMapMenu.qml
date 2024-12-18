import QtQuick
import QtQuick.Controls
import ".."

MenuBase {
	id: random_map_menu
	title: qsTr("Random Map")
	//background: kobold.defines.default_menu_background_file
	
	property int generation_count: 0
	property var selected_era: null
	readonly property var selected_country: diplomatic_map.selected_country
	
	Rectangle {
		id: diplomatic_map_background
		anchors.horizontalCenter: diplomatic_map.horizontalCenter
		anchors.verticalCenter: diplomatic_map.verticalCenter
		width: diplomatic_map.width + 2 * scale_factor
		height: diplomatic_map.height + 2 * scale_factor
		color: Qt.rgba(0.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0, 1)
		border.color: "white"
		border.width: 1 * scale_factor
	}
	
	DiplomaticMap {
		id: diplomatic_map
		anchors.left: era_list.right
		anchors.leftMargin: 16 * scale_factor
		anchors.right: political_map_mode_button.left
		anchors.top: title_item.bottom
		anchors.topMargin: 32 * scale_factor
		anchors.bottom: country_text.top
		anchors.bottomMargin: 16 * scale_factor
		width: 512 * scale_factor
	}
	
	IconButton {
		id: political_map_mode_button
		anchors.top: diplomatic_map_background.top
		anchors.right: parent.right
		anchors.rightMargin: 16 * scale_factor
		icon_identifier: "flag"
		border_color: "white"
		unrounded_left_corners: true
		tooltip: "Political Map Mode"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Political
		}
	}
	
	IconButton {
		id: terrain_map_mode_button
		anchors.top: political_map_mode_button.bottom
		anchors.right: political_map_mode_button.right
		icon_identifier: "mountains"
		border_color: "white"
		unrounded_left_corners: true
		tooltip: "Terrain Map Mode"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Terrain
		}
	}
	
	IconButton {
		id: cultural_map_mode_button
		anchors.top: terrain_map_mode_button.bottom
		anchors.right: political_map_mode_button.right
		icon_identifier: "music"
		border_color: "white"
		unrounded_left_corners: true
		tooltip: "Cultural Map Mode"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Cultural
		}
	}
	
	IconButton {
		id: religious_map_mode_button
		anchors.top: cultural_map_mode_button.bottom
		anchors.right: political_map_mode_button.right
		icon_identifier: "wooden_cross"
		border_color: "white"
		unrounded_left_corners: true
		tooltip: "Religious Map Mode"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Religious
		}
	}
	
	SmallText {
		id: country_text
		text: selected_country ? (
			selected_country.game_data.name
			+ "\n"
			+ "\n" + selected_country.game_data.type_name
			+ (selected_country.game_data.overlord ? (
				"\n" + selected_country.game_data.subject_type.name + " of " + selected_country.game_data.overlord.name
			) : "")
			+ "\n" + selected_country.game_data.title_name
			+ (selected_country.game_data.anarchy ? "\nAnarchy" : "")
			+ (selected_country.great_power && !selected_country.game_data.anarchy ? ("\nScore: " + number_string(selected_country.game_data.score) + " (#" + (selected_country.game_data.score_rank + 1) + ")") : "")
			+ get_subject_type_counts_string(selected_country.game_data.subject_type_counts)
			+ "\n" + selected_country.game_data.provinces.length + " " + (selected_country.game_data.provinces.length > 1 ? "Provinces" : "Province")
			+ get_resource_counts_string(selected_country.game_data.resource_counts)
		) : ""
		anchors.left: diplomatic_map.left
		anchors.leftMargin: 4 * scale_factor
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 4 * scale_factor
		height: 128 * scale_factor
		
		function get_subject_type_counts_string(subject_type_counts) {
			var str = "";
			
			for (const kv_pair of subject_type_counts) {
				var subject_type = kv_pair.key
				var count = kv_pair.value
				str += "\n" + count + " " + get_plural_form(subject_type.name)
			}
			
			return str
		}
		
		function get_resource_counts_string(resource_counts) {
			var str = "";
			
			for (const kv_pair of resource_counts) {
				var resource = kv_pair.key
				var count = kv_pair.value
				str += "\n" + count + " " + resource.name
			}
			
			return str
		}
	}
	
	Rectangle {
		id: era_list_border
		anchors.horizontalCenter: era_list.horizontalCenter
		anchors.verticalCenter: era_list.verticalCenter
		width: era_list.width + 2
		height: era_list.height + 2
		color: "transparent"
		border.color: "white"
		border.width: 1
	}
	
	ListView {
		id: era_list
		anchors.left: parent.left
		anchors.leftMargin: 16 * scale_factor
		anchors.top: title_item.bottom
		anchors.topMargin: 32 * scale_factor
		width: contentItem.childrenRect.width
		height: contentItem.childrenRect.height
		boundsBehavior: Flickable.StopAtBounds
		clip: true
		model: kobold.get_eras()
		delegate: Rectangle {
			width: 256 * scale_factor
			height: visible ? 16 * scale_factor : 0
			visible: !model.modelData.hidden
			color: (selected_era == model.modelData) ? "olive" : "black"
			border.color: "white"
			border.width: 1
			
			SmallText {
				text: model.modelData.name
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.verticalCenter: parent.verticalCenter
			}
			
			MouseArea {
				anchors.fill: parent
				
				onClicked: {
					if (selected_era === model.modelData) {
						return
					}
					
					selected_era = model.modelData
					kobold.game.create_random_map(kobold.get_map_template("earth_random"), selected_era)
				}
			}
		}
	}
	
	TextButton {
		id: start_game_button
		anchors.horizontalCenter: era_list.horizontalCenter
		anchors.bottom: previous_menu_button.top
		anchors.bottomMargin: 8 * scale_factor
		text: qsTr("Start Game")
		width: 96 * scale_factor
		height: 24 * scale_factor
		allowed: selected_country !== null && selected_country.great_power && !selected_country.game_data.anarchy
		tooltip: allowed ? "" : small_text(
			selected_country === null ? "You must select a country to play" : (
				!selected_country.great_power ? "You cannot play as a Minor Nation" : "You cannot play as a country under anarchy"
			)
		)
		
		onClicked: {
			kobold.game.player_country = selected_country
			kobold.game.player_character = selected_country.game_data.ruler
			kobold.game.start()
		}
	}
	
	TextButton {
		id: previous_menu_button
		anchors.horizontalCenter: start_game_button.horizontalCenter
		anchors.bottom: diplomatic_map.bottom
		anchors.bottomMargin: 8 * scale_factor
		text: qsTr("Previous Menu")
		width: 96 * scale_factor
		height: 24 * scale_factor
		
		onClicked: {
			menu_stack.pop()
		}
	}
	
	Connections {
		target: kobold.game
		function onSetup_finished() {
			diplomatic_map.ocean_suffix = random_map_menu.generation_count
			
			if (selected_country !== null && !kobold.game.countries.includes(selected_country)) {
				diplomatic_map.selected_country = null
			}
			
			diplomatic_map.country_suffix = random_map_menu.generation_count
			
			random_map_menu.generation_count += 1
		}
	}
	
	Component.onCompleted: {
		//get the first non-hidden era
		for (var era of kobold.get_eras()) {
			if (!era.hidden) {
				selected_era = era
				break
			}
		}
		
		kobold.game.create_random_map(kobold.get_map_template("earth_random"), selected_era).then(() => {
			diplomatic_map.selected_country = kobold.game.great_powers[random(kobold.game.great_powers.length)]
			diplomatic_map.center_on_selected_country_capital()
		})
	}
}
