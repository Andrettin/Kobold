import QtQuick
import QtQuick.Controls
import "./dialogs"

Item {
	id: politics_view
	
	enum Mode {
		Offices,
		Government,
		Traditions
	}
	
	property var country: null
	readonly property var country_game_data: country ? country.game_data : null
	readonly property var ruler: country_game_data ? country_game_data.ruler : null
	property var new_tradition: null
	property string status_text: ""
	property string middle_status_text: ""
	
	Rectangle {
		id: background
		anchors.top: top_bar.bottom
		anchors.bottom: status_bar.top
		anchors.left: infopanel.right
		anchors.right: button_panel.left
		color: "black"
	}
	
	OfficesView {
		id: offices_view
		anchors.top: top_bar.bottom
		anchors.bottom: status_bar.top
		anchors.left: infopanel.right
		anchors.right: button_panel.left
		visible: politics_view_mode === PoliticsView.Mode.Offices
	}
	
	GovernmentView {
		id: government_view
		anchors.top: top_bar.bottom
		anchors.bottom: status_bar.top
		anchors.left: infopanel.right
		anchors.right: button_panel.left
		visible: politics_view_mode === PoliticsView.Mode.Government
	}
	
	TraditionsView {
		id: traditions_view
		anchors.top: top_bar.bottom
		anchors.bottom: status_bar.top
		anchors.left: infopanel.right
		anchors.right: button_panel.left
		visible: politics_view_mode === PoliticsView.Mode.Traditions
	}
		
	PoliticsButtonPanel {
		id: button_panel
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.right: parent.right
	}
	
	TraditionsInfoPanel {
		id: infopanel
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
	}
	
	StatusBar {
		id: status_bar
		anchors.bottom: parent.bottom
		anchors.left: infopanel.right
		anchors.right: button_panel.left
	}
	
	TopBar {
		id: top_bar
		anchors.top: parent.top
		anchors.left: infopanel.right
		anchors.right: button_panel.left
	}
}
