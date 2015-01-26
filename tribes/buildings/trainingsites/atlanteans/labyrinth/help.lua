-- The Atlantean Labyrinth
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
	func = function(building_description)
	-- need to get this again, so the building description will be of type "trainingsite"
	local building_description = wl.Game():get_building_description(building_description.name)
	return

		--General Section
		building_help_general_string(building_description) ..

		--Dependencies
		-- We would need to parse the production programs to automate the parameters here; so we do it manually
		dependencies_training("atlanteans", building_description,
			"untrained", "fulltrained-attack", "untrained+attack", "fulltrained") ..

		rt(h3(_"Defense Training:")) ..
		dependencies_training_food({{"smoked_fish", "smoked_meat"}, {"bread_atlanteans"}}) ..
		dependencies_training_weapons(building_description, "and",
			{"shield_steel", "shield_advanced"}, "atlanteans_armorsmithy") ..

		rt(h3(_"Evade Training:")) ..
		dependencies_training_food({{"smoked_fish", "smoked_meat"}}) ..

		rt(h3(_"Health Training:")) ..
		dependencies_training_food({{"smoked_fish", "smoked_meat"}, {"bread_atlanteans"}}) ..
		dependencies_training_weapons(building_description, "and",
			{"tabard_golden"}, "atlanteans_weaving_mill") ..

		--Workers Section
		building_help_crew_string(building_description) ..

		--Building Section
		building_help_building_section(building_description) ..

		--Production Section
		building_help_production_section()
	end
}
