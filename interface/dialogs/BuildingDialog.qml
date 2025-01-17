import QtQuick
import QtQuick.Controls
import ".."

DialogBase {
	id: building_dialog
	title: building ? building.name : ""
	width: Math.max(content_column.width + 8 * scale_factor * 2, 256 * scale_factor)
	height: content_column.y + content_column.height + 8 * scale_factor
	
	property var building_slot: null
	readonly property var building: building_slot ? building_slot.building : null
	readonly property int capacity: building_slot ? building_slot.capacity : 0
	readonly property string modifier_string: building_slot ? building_slot.country_modifier_string : 0
	
	ExpandBuildingButton {
		id: expand_building_button
		anchors.top: parent.top
		anchors.topMargin: 16 * scale_factor
		anchors.right: parent.right
		anchors.rightMargin: 16 * scale_factor
	}
	
	UpgradeBuildingButton {
		id: upgrade_building_button
		anchors.top: parent.top
		anchors.topMargin: 16 * scale_factor
		anchors.right: parent.right
		anchors.rightMargin: 16 * scale_factor
	}
	
	Column {
		id: content_column
		anchors.top: title_item.bottom
		anchors.topMargin: 16 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		spacing: 16 * scale_factor
		
		SmallText {
			id: built_label
			anchors.horizontalCenter: parent.horizontalCenter
			text: building && building.provincial ? ("Built: " + get_settlement_building_count(building) + "/" + country_game_data.provinces.length) : ""
			visible: building && building.provincial
		}
		
		SmallText {
			id: modifier_label
			anchors.horizontalCenter: parent.horizontalCenter
			text: format_text(modifier_string)
			visible: building && modifier_string.length > 0 && !building.warehouse && building_slot.available_production_types.length === 0
		}
		
		SmallText {
			id: storage_capacity_label
			anchors.horizontalCenter: parent.horizontalCenter
			text: "Storage Capacity: " + number_string(country_game_data.storage_capacity)
			visible: building && building.warehouse
		}
		
		SmallText {
			id: capacity_label
			anchors.horizontalCenter: parent.horizontalCenter
			text: "Capacity: " + capacity
			visible: capacity > 0
		}
		
		Grid {
			id: commodities_grid
			anchors.horizontalCenter: parent.horizontalCenter
			columns: 4
			visible: building && building.warehouse
			
			Repeater {
				model: country_game_data.stored_commodities
				
				Item {
					width: 64 * scale_factor
					height: 64 * scale_factor
					visible: !commodity.abstract
					
					readonly property var commodity: model.modelData.key
					readonly property int stored: model.modelData.value
					
					Image {
						id: commodity_icon
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.horizontalCenterOffset: -8 * scale_factor
						source: "image://icon/" + commodity.icon.identifier
					}
					
					SmallText {
						id: stored_label
						text: number_string(stored)
						anchors.left: commodity_icon.right
						anchors.leftMargin: 4 * scale_factor
						anchors.bottom: commodity_icon.bottom
					}
					
					MouseArea {
						anchors.fill: commodity_icon
						hoverEnabled: true
						
						onEntered: {
							status_text = commodity.name
						}
						
						onExited: {
							status_text = ""
						}
					}
				}
			}
		}
		
		Repeater {
			model: building_slot ? building_slot.available_production_types : []
			
			Item {
				width: production_slider.width + 16 * scale_factor + production_formula_row.width
				height: production_formula_row.height
				
				readonly property var production_type: model.modelData
				readonly property var output_commodity: production_type.output_commodity
				property int employed_capacity: building_slot.get_production_type_employed_capacity(production_type)
				property int output_value: building_slot.get_production_type_output(production_type)
				
				Row {
					id: production_formula_row
					anchors.verticalCenter: parent.verticalCenter
					anchors.left: parent.left
					
					Repeater {
						model: production_type.input_commodities
						
						Row {
							readonly property var input_commodity: model.modelData.key
							readonly property int input_value: model.modelData.value
							
							Item {
								width: 32 * scale_factor
								height: 32 * scale_factor
								visible: index > 0
								
								NormalText {
									text: "+"
									anchors.verticalCenter: parent.verticalCenter
									anchors.horizontalCenter: parent.horizontalCenter
								}
							}
							
							Repeater {
								model: input_value
								
								Image {
									id: input_commodity_icon
									source: "image://icon/" + input_commodity.icon.identifier
								}
							}
						}
					}
					
					Item {
						width: 32 * scale_factor
						height: 32 * scale_factor
						
						NormalText {
							text: "→"
							anchors.verticalCenter: parent.verticalCenter
							anchors.horizontalCenter: parent.horizontalCenter
						}
					}
					
					Image {
						id: output_commodity_icon
						source: "image://icon/" + output_commodity.icon.identifier
					}
				}
				
				MouseArea {
					anchors.fill: production_formula_row
					hoverEnabled: true
					
					onEntered: {
						status_text = get_production_formula_string(production_type)
					}
					
					onExited: {
						status_text = ""
					}
				}
				
				CustomSlider {
					id: production_slider
					anchors.verticalCenter: production_formula_row.verticalCenter
					anchors.right: parent.right
					value: employed_capacity
					secondary_value: output_value
					max_value: capacity
					
					onDecremented: {
						if (building_slot.can_decrease_production(production_type)) {
							building_slot.decrease_production(production_type)
							employed_capacity = building_slot.get_production_type_employed_capacity(production_type)
							output_value = building_slot.get_production_type_output(production_type)
						}
					}
					
					onIncremented: {
						if (building_slot.can_increase_production(production_type)) {
							building_slot.increase_production(production_type)
							employed_capacity = building_slot.get_production_type_employed_capacity(production_type)
							output_value = building_slot.get_production_type_output(production_type)
						}
					}
					
					onClicked: function(target_value) {
						var current_employed_capacity = employed_capacity
						
						if (target_value > current_employed_capacity) {
							while (target_value > current_employed_capacity) {
								if (!building_slot.can_increase_production(production_type)) {
									break
								}
								
								building_slot.increase_production(production_type)
								current_employed_capacity = building_slot.get_production_type_employed_capacity(production_type)
							}
						} else if (target_value < current_employed_capacity) {
							while (target_value < current_employed_capacity) {
								if (!building_slot.can_decrease_production(production_type)) {
									break
								}
								
								building_slot.decrease_production(production_type)
								current_employed_capacity = building_slot.get_production_type_employed_capacity(production_type)
							}
						} else {
							return
						}
						
						employed_capacity = current_employed_capacity
						output_value = building_slot.get_production_type_output(production_type)
						update_status_text()
					}
					
					onEntered: {
						update_status_text()
					}
					
					onExited: {
						status_text = ""
					}
					
					function update_status_text() {
						var text = ""
						
						var base_input_commodities = production_type.input_commodities
						var input_commodities = building_slot.get_production_type_inputs(production_type)
						
						for (var i = 0; i < input_commodities.length; i++) {
							var commodity = input_commodities[i].key
							var quantity = input_commodities[i].value
							var base_quantity = base_input_commodities[i].value * employed_capacity
							
							if (text.length > 0) {
								text += " + "
							}
							
							if (quantity !== base_quantity) {
								text += "("
							}
							
							text += base_quantity + " " + commodity.name
							
							if (quantity !== base_quantity) {
								var modifier = Math.floor(100 * 100 / (100 + country_game_data.throughput_modifier + country_game_data.get_commodity_throughput_modifier(output_commodity)) - 100)
								text += " " + (modifier > 0 ? "+" : "-") + " " + Math.abs(modifier) + "% = " + quantity + " " + commodity.name 										
								text += ")"
							}
						}
						
						text += " → " + employed_capacity + " " + output_commodity.name
						
						if (output_value !== employed_capacity) {
							var modifier = country_game_data.output_modifier + country_game_data.get_commodity_output_modifier(output_commodity)
							if (production_type.industrial) {
								modifier += country_game_data.industrial_output_modifier
							}
							text += " " + (modifier > 0 ? "+" : "-") + " " + Math.abs(modifier) + "% = " + output_value + " " + output_commodity.name
						}
						
						status_text = text
					}
				}
			}
		}
		
		TextButton {
			id: ok_button
			anchors.horizontalCenter: parent.horizontalCenter
			text: "OK"
			onClicked: {
				building_dialog.close()
			}
		}
	}
	
	function get_settlement_building_count(building) {
		var count = country_game_data.get_settlement_building_count(building)
		
		//also count the best non-capital building immediately below the capital-specific one
		while (building && (building.capital_only || building.provincial_capital_only)) {
			building = building.required_building
			count += country_game_data.get_settlement_building_count(building)
		}
		
		return count
	}
	
	function get_production_formula_string(production_type) {
		var str = ""
		
		var input_commodities = production_type.input_commodities
		
		for (var kv_pair of input_commodities) {
			var commodity = kv_pair.key
			var quantity = kv_pair.value
			
			if (str.length > 0) {
				str += " + "
			}
			
			str += quantity + " " + commodity.name
		}
		
		str += " makes " + production_type.output_value + " " + production_type.output_commodity.name
		
		return str
	}
}
