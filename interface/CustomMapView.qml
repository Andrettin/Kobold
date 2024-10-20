import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning
import map_country_model 1.0
import map_grid_model 1.0
import map_province_model 1.0
import map_site_model 1.0
import "./dialogs"

Item {
	id: map_view
	
	property string status_text: ""
	property string middle_status_text: ""
	property string interface_style: "dwarven"
	readonly property int tile_size: kobold.defines.tile_size.width * scale_factor
	readonly property real map_area_start_x: map.contentX / tile_size
	readonly property real map_area_start_y: map.contentY / tile_size
	readonly property real map_area_tile_width: map.width / tile_size
	readonly property real map_area_tile_height: map.height / tile_size
	
	property var selected_civilian_unit: null
	property var selected_site: null
	property var selected_province: null
	property bool selected_garrison: false
	
	property bool tile_detail_mode: false
	
	readonly property var event_dialog_component: Qt.createComponent("dialogs/EventDialog.qml")
	readonly property var notification_dialog_component: Qt.createComponent("dialogs/NotificationDialog.qml")
	property var dialogs: []
	
	Rectangle {
		id: map_view_background
		color: "black"
		anchors.fill: parent
	}
	
	TableView {
		id: map
		anchors.top: top_bar.bottom
		anchors.bottom: status_bar.top
		anchors.left: infopanel.right
		anchors.right: right_bar.left
		leftMargin: 0
		rightMargin: 0
		topMargin: 0
		bottomMargin: 0
		contentWidth: tile_size * kobold.map.width
		contentHeight: tile_size * kobold.map.height
		boundsBehavior: Flickable.StopAtBounds
		clip: true
		visible: true
		model: MapGridModel {}
		delegate: TileView {}
		
		function pixel_to_tile_pos(pixel_x, pixel_y) {
			var tile_x = Math.floor(pixel_x / tile_size)
			var tile_y = Math.floor(pixel_y / tile_size)
			
			return Qt.point(tile_x, tile_y)
		}
		
		function center_on_tile(tile_x, tile_y) {
			var pixel_x = tile_x * tile_size - map.width / 2
			var pixel_y = tile_y * tile_size - map.height / 2
			
			map.contentX = Math.min(Math.max(pixel_x, 0), map.contentWidth - map.width)
			map.contentY = Math.min(Math.max(pixel_y, 0), map.contentHeight - map.height)
		}
		
		function center_on_country_capital(country) {
			var capital = country.game_data.capital
			
			if (capital === null) {
				return
			}
			
			var capital_game_data = capital.game_data
			var capital_x = capital_game_data.tile_pos.x
			var capital_y = capital_game_data.tile_pos.y
			
			center_on_tile(capital_x, capital_y)
		}
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
		anchors.left: menu_button_bar.right
		anchors.right: right_bar.left
	}
	
	MenuButtonBar {
		id: menu_button_bar
		anchors.top: parent.top
		anchors.left: parent.left
	}
	
	Rectangle {
		id: minimap_area
		anchors.top: menu_button_bar.bottom
		anchors.left: parent.left
		color: interface_background_color
		width: 176 * scale_factor
		height: minimap_borders.height
		
		Rectangle {
			color: "gray"
			anchors.top: parent.top
			anchors.bottom: parent.bottom
			anchors.right: parent.right
			width: 1 * scale_factor
		}
		
		Rectangle {
			id: minimap_borders
			anchors.top: minimap.top
			anchors.topMargin: -1 * scale_factor
			anchors.bottom: minimap.bottom
			anchors.bottomMargin: -1 * scale_factor
			anchors.left: minimap.left
			anchors.leftMargin: -1 * scale_factor
			anchors.right: minimap.right
			anchors.rightMargin: -1 * scale_factor
			color: "gray"
		}
		
		Minimap {
			id: minimap
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter
		}
	}
	
	InfoPanel {
		id: infopanel
		anchors.top: minimap_area.bottom
		anchors.bottom: parent.bottom
		anchors.left: parent.left
	}
	
	TraditionChoiceDialog {
		id: tradition_choice_dialog
	}
	
	BeliefChoiceDialog {
		id: belief_choice_dialog
	}
	
	ModifierDialog {
		id: modifier_dialog
	}
	
	GarrisonDialog {
		id: garrison_dialog
	}
	
	Keys.onPressed: function(event) {
		switch (event.key) {
			case Qt.Key_E:
				infopanel.end_turn_button.down = true
				break
			case Qt.Key_Shift:
				tile_detail_mode = true
				break
		}
	}
	
	Keys.onReleased: function(event) {
		if (event.isAutoRepeat) {
			return
		}
		
		switch (event.key) {
			case Qt.Key_E:
				infopanel.end_turn_button.down = undefined
				infopanel.end_turn_button.onReleased()
				break
			case Qt.Key_Shift:
				tile_detail_mode = false
				break
		}
	}
	
	Connections {
		target: kobold
		
		function onNotification_added(title, portrait_object, text) {
			if (notification_dialog_component.status == Component.Error) {
				console.error(notification_dialog_component.errorString())
				return
			}
			
			var dialog = notification_dialog_component.createObject(map_view, {
				title: title,
				portrait_object: portrait_object,
				text: text
			})
			
			dialog.open()
		}
		
		function onEvent_fired(event_instance) {
			if (event_dialog_component.status == Component.Error) {
				console.error(event_dialog_component.errorString())
				return
			}
			
			var event_dialog = event_dialog_component.createObject(map_view, {
				event_instance: event_instance,
				interface_style: map_view.interface_style
			})
			
			event_dialog.open()
		}
	}
	
	Connections {
		target: kobold.game.player_country.game_data
		
		function onJournal_entry_completed(journal_entry) {
			if (notification_dialog_component.status == Component.Error) {
				console.error(notification_dialog_component.errorString())
				return
			}
			
			var dialog = notification_dialog_component.createObject(map_view, {
				title: journal_entry.name,
				portrait_object: journal_entry.portrait,
				text: journal_entry.description,
				effects_text: journal_entry.get_completion_effects_string(kobold.game.player_country)
			})
			
			dialog.open()
		}
	}
	
	onSelected_garrisonChanged: {
		kobold.clear_selected_military_units()
	}
	
	Component.onCompleted: {
		map.center_on_country_capital(kobold.game.player_country)
	}
}
