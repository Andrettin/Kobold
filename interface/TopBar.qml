import QtQuick
import QtQuick.Controls

Rectangle {
	id: top_bar
	color: interface_background_color
	height: 16 * scale_factor
	
	Rectangle {
		color: "gray"
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		height: 1 * scale_factor
		z: 1 //draw on top of everything else
	}
	
	readonly property var stored_commodities: kobold.game.player_country.game_data.stored_commodities
	readonly property var wealth_value: kobold.game.player_country.game_data.stored_commodities.length > 0 ? kobold.game.player_country.game_data.get_stored_commodity(kobold.defines.wealth_commodity) : 0 //refer to the stored commodities to ensure the counter is updated when wealth changes
	readonly property var wealth_unit: kobold.defines.wealth_commodity.get_unit(wealth_value)
	
	SmallText {
		id: date_label
		text: kobold.game.date_string
		anchors.top: parent.top
		anchors.topMargin: 1 * scale_factor
		anchors.left: parent.left
		anchors.leftMargin: 16 * scale_factor
		
		MouseArea {
			anchors.fill: parent
			hoverEnabled: true
			onEntered: {
				status_text = "Current Month and Year"
			}
			onExited: {
				status_text = ""
			}
		}
	}
	
	Image {
		id: wealth_icon
		source: "image://icon/trade_consulate"
		anchors.top: parent.top
		anchors.topMargin: 3 * scale_factor
		anchors.left: date_label.left
		anchors.leftMargin: 128 * scale_factor
	}

	SmallText {
		id: wealth_label
		text: number_string(Math.floor(wealth_value / kobold.defines.wealth_commodity.get_unit_value(wealth_unit))) + " " + wealth_unit.suffix
		anchors.top: parent.top
		anchors.topMargin: 1 * scale_factor
		anchors.left: wealth_icon.right
		anchors.leftMargin: 4 * scale_factor
	}

	MouseArea {
		anchors.top: wealth_icon.top
		anchors.bottom: wealth_icon.bottom
		anchors.left: wealth_icon.left
		anchors.right: wealth_label.right
		hoverEnabled: true
		onEntered: {
			status_text = get_plural_form(wealth_unit.name)
		}
		onExited: {
			status_text = ""
		}
	}
	
	Image {
		id: prestige_icon
		source: "image://icon/non_aggression_pact_shield"
		anchors.top: parent.top
		anchors.topMargin: 3 * scale_factor
		anchors.left: wealth_label.left
		anchors.leftMargin: 96 * scale_factor
	}

	SmallText {
		id: prestige_label
		text: kobold.game.player_country.game_data.stored_commodities.length > 0 ? number_string(kobold.game.player_country.game_data.get_stored_commodity(kobold.defines.prestige_commodity)) : 0 //refer to the stored commodities to ensure the counter is updated when prestige changes
		anchors.top: parent.top
		anchors.topMargin: 1 * scale_factor
		anchors.left: prestige_icon.right
		anchors.leftMargin: 2 * scale_factor
	}

	MouseArea {
		anchors.top: prestige_icon.top
		anchors.bottom: prestige_icon.bottom
		anchors.left: prestige_icon.left
		anchors.right: prestige_label.right
		hoverEnabled: true
		onEntered: {
			status_text = "Prestige"
		}
		onExited: {
			status_text = ""
		}
	}
}
