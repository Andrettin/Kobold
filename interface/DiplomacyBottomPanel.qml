import QtQuick
import QtQuick.Controls

Rectangle {
	id: bottom_panel
	color: interface_background_color
	height: 192 * scale_factor
	
	readonly property var selected_country_ruler: selected_country_game_data ? selected_country_game_data.ruler : null
	
	Rectangle {
		color: "gray"
		anchors.left: parent.left
		anchors.leftMargin: 15 * scale_factor
		anchors.right: parent.right
		anchors.rightMargin: 15 * scale_factor
		anchors.top: parent.top
		height: 1 * scale_factor
	}
	
	IconButton {
		id: political_map_mode_button
		anchors.top: parent.top
		anchors.topMargin: 8 * scale_factor
		anchors.right: parent.right
		anchors.rightMargin: 8 * scale_factor
		icon_identifier: "flag"
		border_color: diplomatic_map.mode === DiplomaticMap.Mode.Political ? "white" : "gray"
		tooltip: "Political Map"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Political
		}
	}
	
	IconButton {
		id: diplomatic_map_mode_button
		anchors.top: political_map_mode_button.bottom
		anchors.topMargin: 4 * scale_factor
		anchors.right: political_map_mode_button.right
		icon_identifier: "treaty"
		border_color: diplomatic_map.mode === DiplomaticMap.Mode.Treaty ? "white" : "gray"
		tooltip: "Treaty Map"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Treaty
			diplomatic_map_view.selected_diplomacy_state = -1
			diplomatic_map_view.selected_consulate = null
		}
	}
	
	IconButton {
		id: terrain_map_mode_button
		anchors.top: diplomatic_map_mode_button.bottom
		anchors.topMargin: 4 * scale_factor
		anchors.right: political_map_mode_button.right
		icon_identifier: "mountains"
		border_color: diplomatic_map.mode === DiplomaticMap.Mode.Terrain ? "white" : "gray"
		tooltip: "Terrain Map"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Terrain
		}
	}
	
	IconButton {
		id: cultural_map_mode_button
		anchors.top: terrain_map_mode_button.bottom
		anchors.topMargin: 4 * scale_factor
		anchors.right: political_map_mode_button.right
		icon_identifier: "music"
		border_color: diplomatic_map.mode === DiplomaticMap.Mode.Cultural ? "white" : "gray"
		tooltip: "Cultural Map"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Cultural
		}
	}
	
	IconButton {
		id: religious_map_mode_button
		anchors.top: cultural_map_mode_button.top
		anchors.right: cultural_map_mode_button.left
		anchors.rightMargin: 4 * scale_factor
		icon_identifier: "wooden_cross"
		border_color: diplomatic_map.mode === DiplomaticMap.Mode.Religious ? "white" : "gray"
		tooltip: "Religious Map"
		
		onReleased: {
			diplomatic_map.mode = DiplomaticMap.Mode.Religious
		}
	}
	
	SmallText {
		id: country_text
		text: format_text(selected_country && selected_country_game_data ? (
			selected_country_game_data.name
			+ "\n"
			+ "\n" + selected_country_game_data.type_name
			+ (selected_country_game_data.overlord ? (
				"\n" + selected_country_game_data.subject_type.name + " of " + selected_country_game_data.overlord.name
			) : "")
			+ "\n" + selected_country_game_data.title_name
			+ (selected_country_game_data.anarchy ? "\nAnarchy" : "")
			+ (selected_country.great_power && !selected_country_game_data.anarchy ? ("\nScore: " + number_string(selected_country_game_data.score) + " (#" + (selected_country_game_data.score_rank + 1) + ")") : "")
			+ "\n" + selected_country_game_data.provinces.length + " " + (selected_country_game_data.provinces.length > 1 ? "Provinces" : "Province")
			+ "\n" + selected_country_game_data.site_count + " " + (selected_country_game_data.site_count > 1 ? "Sites" : "Sites")
			+ "\nLevel: " + selected_country.game_data.level
			+ "\nExperience: " + number_string(selected_country.game_data.experience)
			+ "\nGovernment Type: " + selected_country.game_data.government_type.name
			+ (selected_country.game_data.attribute_values.length > 0 ? ("\nAttributes:\n\t" + values_to_string(selected_country.game_data.attribute_values, "\n\t")) : "")
			+ "\nUnrest: " + Math.max(selected_country.game_data.unrest, 0)
			+ "\nConsumption: " + Math.max(selected_country.game_data.consumption, 0)
			+ (selected_country.game_data.skill_bonuses.length > 0 ? ("\nSkills:\n\t" + values_to_string(selected_country.game_data.skill_bonuses, "\n\t", true)) : "")
			+ (selected_country.game_data.feats.length > 0 ? "\nFeats:\n\t" + string_list_to_string(object_counts_to_name_list(selected_country.game_data.feats), "\n\t") : "")
		) : "")
		anchors.left: bottom_panel.left
		anchors.leftMargin: 16 * scale_factor
		anchors.top: bottom_panel.top
		anchors.topMargin: 16 * scale_factor
		width: 128 * scale_factor
	}
	
	SmallText {
		id: ruler_label
		anchors.top: country_text.top
		anchors.horizontalCenter: ruler_portrait.horizontalCenter
		text: ruler_portrait.character_titled_name
		visible: ruler_portrait.visible
	}
	
	CharacterPortraitButton {
		id: ruler_portrait
		anchors.top: ruler_label.bottom
		anchors.topMargin: 12 * scale_factor
		anchors.left: country_text.right
		anchors.leftMargin: 32 * scale_factor
		character: selected_country_ruler
	}
	
	Row {
		id: diplomatic_actions_row
		anchors.top: parent.top
		anchors.topMargin: 8 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		spacing: 8 * scale_factor
		visible: diplomatic_map.mode === DiplomaticMap.Mode.Treaty && selected_country === null
		
		IconButton {
			id: offer_peace_button
			icon_identifier: "philosophy"
			border_color: diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.Peace ? "white" : "gray"
			tooltip: "Offer Peace"
			
			onReleased: {
				if (diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.Peace) {
					diplomatic_map_view.selected_diplomacy_state = -1
				} else {
					diplomatic_map_view.selected_diplomacy_state = DiplomaticView.DiplomacyState.Peace
					diplomatic_map_view.selected_consulate = null
				}
			}
		}
		
		IconButton {
			id: declare_war_button
			icon_identifier: "crossed_sabers"
			border_color: diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.War ? "white" : "gray"
			tooltip: "Declare War"
			
			onReleased: {
				if (diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.War) {
					diplomatic_map_view.selected_diplomacy_state = -1
				} else {
					diplomatic_map_view.selected_diplomacy_state = DiplomaticView.DiplomacyState.War
					diplomatic_map_view.selected_consulate = null
				}
			}
		}
	}
	
	Row {
		id: alliances_row
		anchors.top: diplomatic_actions_row.bottom
		anchors.topMargin: 8 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		spacing: 8 * scale_factor
		visible: diplomatic_actions_row.visible
		
		IconButton {
			id: offer_pact_button
			icon_identifier: "wall"
			border_color: diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.NonAggressionPact ? "white" : "gray"
			tooltip: "Offer Non-Aggression Pact"
			
			onReleased: {
				if (diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.NonAggressionPact) {
					diplomatic_map_view.selected_diplomacy_state = -1
				} else {
					diplomatic_map_view.selected_diplomacy_state = DiplomaticView.DiplomacyState.NonAggressionPact
					diplomatic_map_view.selected_consulate = null
				}
			}
		}
		
		IconButton {
			id: offer_alliance_button
			icon_identifier: "flag"
			border_color: diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.Alliance ? "white" : "gray"
			tooltip: "Offer Alliance"
			
			onReleased: {
				if (diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.Alliance) {
					diplomatic_map_view.selected_diplomacy_state = -1
				} else {
					diplomatic_map_view.selected_diplomacy_state = DiplomaticView.DiplomacyState.Alliance
					diplomatic_map_view.selected_consulate = null
				}
			}
		}
		
		IconButton {
			id: join_empire_button
			icon_identifier: "crown_imperial"
			border_color: diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.Vassal ? "white" : "gray"
			tooltip: "Invite to Join Empire"
			
			onReleased: {
				if (diplomatic_map_view.selected_diplomacy_state === DiplomaticView.DiplomacyState.Vassal) {
					diplomatic_map_view.selected_diplomacy_state = -1
				} else {
					diplomatic_map_view.selected_diplomacy_state = DiplomaticView.DiplomacyState.Vassal
					diplomatic_map_view.selected_consulate = null
				}
			}
		}
	}
	
	Row {
		id: consulates_row
		anchors.top: alliances_row.bottom
		anchors.topMargin: 8 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		spacing: 8 * scale_factor
		visible: diplomatic_actions_row.visible
		
		IconButton {
			id: build_trade_consulate_button
			icon_identifier: "wealth"
			border_color: diplomatic_map_view.selected_consulate === consulate ? "white" : "gray"
			tooltip: "Build a Trade Consulate"
			
			readonly property var consulate: kobold.get_consulate("trade_consulate")
			
			onReleased: {
				if (diplomatic_map_view.selected_consulate === consulate) {
					diplomatic_map_view.selected_consulate = null
				} else {
					diplomatic_map_view.selected_consulate = consulate
					diplomatic_map_view.selected_diplomacy_state = -1
				}
			}
		}
		
		IconButton {
			id: build_embassy_button
			icon_identifier: "treaty"
			border_color: diplomatic_map_view.selected_consulate === consulate ? "white" : "gray"
			tooltip: "Build an Embassy"
			
			readonly property var consulate: kobold.get_consulate("embassy")
			
			onReleased: {
				if (diplomatic_map_view.selected_consulate === consulate) {
					diplomatic_map_view.selected_consulate = null
				} else {
					diplomatic_map_view.selected_consulate = consulate
					diplomatic_map_view.selected_diplomacy_state = -1
				}
			}
		}
	}
	
	TextButton {
		id: back_button
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 8 * scale_factor
		text: qsTr("Back")
		width: 64 * scale_factor
		height: 24 * scale_factor
		
		onClicked: {
			menu_stack.pop()
		}
	}
}
