import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import scenario_model 1.0
import ".."
import "../dialogs"

MenuBase {
	id: scenario_menu
	title: qsTr("Scenario")
	//background: kobold.defines.default_menu_background_file
	
	property var loading_scenario: false
	property var map_template_loaded: false
	property var previous_selected_scenario: null
	property var selected_scenario: null
	readonly property var selected_country: diplomatic_map.selected_country
	readonly property var selected_country_game_data: selected_country ? selected_country.game_data : null
	property int setup_count: 0
	readonly property var scenarios: kobold.get_top_level_scenarios()
	
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
		anchors.left: scenario_list.right
		anchors.leftMargin: 16 * scale_factor
		anchors.right: political_map_mode_button.left
		anchors.top: title_item.bottom
		anchors.topMargin: 32 * scale_factor
		anchors.bottom: country_text_area.top
		anchors.bottomMargin: 16 * scale_factor
		width: 512 * scale_factor
		visible: map_template_loaded
		enabled: !loading_scenario
	}
	
	LargeText {
		id: loading_scenario_label
		anchors.horizontalCenter: diplomatic_map_background.horizontalCenter
		anchors.verticalCenter: diplomatic_map_background.verticalCenter
		text: "Loading..."
		visible: loading_scenario && !map_template_loaded
	}
	
	IconButton {
		id: political_map_mode_button
		anchors.top: diplomatic_map_background.top
		anchors.right: parent.right
		anchors.rightMargin: 16 * scale_factor
		icon_identifier: "flag"
		border_color: "white"
		unrounded_left_corners: true
		tooltip: "Political Map"
		enabled: !loading_scenario
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Political
		}
	}
	
	IconButton {
		id: diplomatic_map_mode_button
		anchors.top: political_map_mode_button.bottom
		anchors.right: political_map_mode_button.right
		icon_identifier: "treaty"
		border_color: "white"
		unrounded_left_corners: true
		tooltip: "Treaty Map"
		enabled: !loading_scenario
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Treaty
		}
	}
	
	IconButton {
		id: terrain_map_mode_button
		anchors.top: diplomatic_map_mode_button.bottom
		anchors.right: political_map_mode_button.right
		icon_identifier: "mountains"
		border_color: "white"
		unrounded_left_corners: true
		tooltip: "Terrain Map"
		enabled: !loading_scenario
		
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
		tooltip: "Cultural Map"
		enabled: !loading_scenario
		
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
		tooltip: "Religious Map"
		enabled: !loading_scenario
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Religious
		}
	}
	
	SmallText {
		id: scenario_text
		text: selected_scenario ? (
			(selected_scenario.timeline ? ("Timeline: " + selected_scenario.timeline.name + "\n\n") : "")
			+ selected_scenario.description
		) : ""
		anchors.left: scenario_list.left
		anchors.leftMargin: 4 * scale_factor
		anchors.right: scenario_list.right
		anchors.rightMargin: 4 * scale_factor
		anchors.top: country_text_area.top
		wrapMode: Text.WordWrap
	}
	
	Flickable {
		id: country_text_area
		anchors.left: diplomatic_map.left
		anchors.leftMargin: 4 * scale_factor
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 4 * scale_factor
		width: 192 * scale_factor
		height: 128 * scale_factor
		contentWidth: contentItem.childrenRect.width
		contentHeight: contentItem.childrenRect.height
		boundsBehavior: Flickable.StopAtBounds
		clip: true
		
		SmallText {
			id: country_text
			text: selected_country ? format_text(
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
				+ "\n" + selected_country.game_data.site_count + " " + (selected_country.game_data.site_count > 1 ? "Sites" : "Sites")
				+ "\nLevel: " + selected_country.game_data.level
				+ "\nExperience: " + number_string(selected_country.game_data.experience)
				+ "\nGovernment Type: " + selected_country.game_data.government_type.name
				+ (selected_country.game_data.attribute_values.length > 0 ? ("\nAttributes:\n\t" + values_to_string(selected_country.game_data.attribute_values, 0, "\n\t")) : "")
				+ "\nUnrest: " + Math.max(selected_country.game_data.unrest, 0)
				+ "\nConsumption: " + Math.max(selected_country.game_data.consumption, 0)
				+ (selected_country.game_data.skill_bonuses.length > 0 ? ("\nSkills:\n\t" + values_to_string(selected_country.game_data.skill_bonuses, 0, "\n\t", true)) : "")
				+ (selected_country.game_data.feats.length > 0 ? "\nFeats:\n\t" + string_list_to_string(object_counts_to_name_list(selected_country.game_data.feats), "\n\t") : "")
			) : ""
			
			function get_subject_type_counts_string(subject_type_counts) {
				var str = "";
				
				for (const kv_pair of subject_type_counts) {
					var subject_type = kv_pair.key
					var count = kv_pair.value
					str += "\n" + count + " " + (count > 1 ? get_plural_form(subject_type.name) : subject_type.name)
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
	}
	
	SmallText {
		id: ruler_label
		anchors.top: country_text_area.top
		anchors.horizontalCenter: ruler_portrait.horizontalCenter
		text: ruler_portrait.character_titled_name
		visible: ruler_portrait.visible
	}
	
	CharacterPortraitButton {
		id: ruler_portrait
		anchors.top: ruler_label.bottom
		anchors.topMargin: 12 * scale_factor
		anchors.left: country_text_area.right
		anchors.leftMargin: 32 * scale_factor
	}
	
	Rectangle {
		id: scenario_list_border
		anchors.horizontalCenter: scenario_list.horizontalCenter
		anchors.verticalCenter: scenario_list.verticalCenter
		width: scenario_list.width + 2
		height: scenario_list.height + 2
		color: "transparent"
		border.color: "white"
		border.width: 1
	}
	
	TreeView {
		id: scenario_list
		anchors.left: parent.left
		anchors.leftMargin: 16 * scale_factor
		anchors.top: title_item.bottom
		anchors.topMargin: 32 * scale_factor
		anchors.bottom: start_game_button.top
		anchors.bottomMargin: 16 * scale_factor
		width: 256 * scale_factor
		height: 128 * scale_factor
		boundsBehavior: Flickable.StopAtBounds
		clip: true
		model: ScenarioModel {}
		delegate: TreeViewDelegate {
			implicitWidth: scenario_list.width
			font.family: berenika_font.name
			font.pixelSize: 10 * scale_factor
			current: selected_scenario == model.scenario
			selected: selected_scenario == model.scenario
			Material.theme: Material.Dark
			Material.accent: "olive"
			
			onClicked: {
				if (selected_scenario === model.scenario || loading_scenario) {
					return
				}
				
				loading_scenario = true
				previous_selected_scenario = selected_scenario
				selected_scenario = model.scenario
				if (previous_selected_scenario === null || previous_selected_scenario.map_template !== selected_scenario.map_template) {
					map_template_loaded = false
				}
				kobold.game.setup_scenario(selected_scenario).then(() => {
					if (previous_selected_scenario === null || previous_selected_scenario.map_template !== selected_scenario.map_template) {
						select_random_country()
					}
					update_selected_country_data(diplomatic_map.selected_country)
					loading_scenario = false
					map_template_loaded = true
				})
			}
		}

		onModelChanged: {
			adjust_scenario_list_position(scenarios.indexOf(selected_scenario))
		}
	}
	
	TextButton {
		id: start_game_button
		anchors.horizontalCenter: scenario_list.horizontalCenter
		anchors.bottom: game_rules_button.top
		anchors.bottomMargin: 8 * scale_factor
		text: qsTr("Start Game")
		width: 96 * scale_factor
		height: 24 * scale_factor
		enabled: !loading_scenario
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
		id: game_rules_button
		anchors.horizontalCenter: start_game_button.horizontalCenter
		anchors.bottom: previous_menu_button.top
		anchors.bottomMargin: 8 * scale_factor
		text: qsTr("Game Rules")
		width: 96 * scale_factor
		height: 24 * scale_factor
		
		onClicked: {
			menu_stack.push("GameRulesMenu.qml", {
				selected_scenario: scenario_menu.selected_scenario
			})
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
	
	CharacterDialog {
		id: character_dialog
	}
	
	Connections {
		target: kobold.game
		function onSetup_finished() {
			if (selected_scenario.map_template.randomly_generated) {
				diplomatic_map.ocean_suffix = scenario_menu.setup_count
			} else if (selected_scenario.map_template.identifier !== diplomatic_map.ocean_suffix) {
				diplomatic_map.ocean_suffix = selected_scenario.map_template.identifier
			}
			
			if (selected_country !== null && !kobold.game.countries.includes(selected_country)) {
				diplomatic_map.selected_country = null
			}
			
			diplomatic_map.country_suffix = scenario_menu.setup_count
			
			scenario_menu.setup_count += 1
		}
	}
	
	Component.onCompleted: {
		select_random_scenario()
	}
	
	function select_random_scenario() {
		loading_scenario = true
		previous_selected_scenario = selected_scenario
		//get a random scenario
		var scenario_index = random(scenarios.length)
		selected_scenario = scenarios[scenario_index]
		
		adjust_scenario_list_position(scenario_index)
		
		kobold.game.setup_scenario(selected_scenario).then(() => {
			select_random_country()
			loading_scenario = false
			map_template_loaded = true
		})
	}
	
	function select_random_country() {
		if (selected_scenario.default_countries.length > 0) {
			diplomatic_map.selected_country = selected_scenario.default_countries[random(selected_scenario.default_countries.length)]
		} else if (kobold.game.great_powers.length > 0) {
			diplomatic_map.selected_country = kobold.game.great_powers[random(kobold.game.great_powers.length)]
		} else {
			diplomatic_map.selected_country = kobold.game.countries[random(kobold.game.countries.length)]
		}
		diplomatic_map.center_on_selected_country_capital()
	}
	
	function adjust_scenario_list_position(scenario_index) {
		if (scenario_index === -1) {
			return
		}
		
		var scenario_rect_top = scenario_index * 20 * scale_factor
		var scenario_rect_bottom = scenario_rect_top + 1 * 20 * scale_factor - 1
		if (scenario_rect_top >= (scenario_list.contentY + scenario_list.height)) {
			scenario_list.contentY = scenario_rect_bottom - scenario_list.height
		} else if (scenario_rect_top < scenario_list.contentY) {
			scenario_list.contentY = scenario_rect_top
		}
	}
	
	function update_selected_country_data(country) {
		ruler_portrait.character = country ? country.game_data.ruler : null
		if (ruler_portrait.character !== null) {
			ruler_portrait.portrait = ruler_portrait.character.game_data.portrait
			ruler_portrait.character_titled_name = ruler_portrait.character.game_data.titled_name
		}
	}
	
	onSelected_countryChanged: {
		update_selected_country_data(selected_country)
	}
}
