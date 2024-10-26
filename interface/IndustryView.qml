import QtQuick
import QtQuick.Controls
import "./dialogs"

Item {
	id: industry_view
	
	property var country: null
	readonly property var country_game_data: country ? country.game_data : null
	property string interface_style: "dwarven"
	property string status_text: ""
	property string middle_status_text: ""
	
	Rectangle {
		id: portrait_grid_view_background
		anchors.top: top_bar.bottom
		anchors.bottom: status_bar.top
		anchors.left: infopanel.right
		anchors.right: right_bar.left
		color: "black"
	}
	
	RightBar {
		id: right_bar
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.right: parent.right
	}
	
	StatusBar {
		id: status_bar
		anchors.bottom: parent.bottom
		anchors.left: infopanel.right
		anchors.right: right_bar.left
	}
	
	TopBar {
		id: top_bar
		anchors.top: parent.top
		anchors.left: infopanel.right
		anchors.right: right_bar.left
	}
	
	IndustryInfoPanel {
		id: infopanel
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
	}
	
	BuildingDialog {
		id: building_dialog
	}
	
	BuildBuildingDialog {
		id: build_building_dialog
	}
}
