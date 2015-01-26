-- The Imperial Training Camp
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
		dependencies_training("empire", building_description,
			"untrained", "fulltrained-evade", "untrained+evade", "fulltrained") ..

		rt(h3(_"Attack Training:")) ..
		dependencies_training_food({{"fish", "meat"}, {"bread_empire"}}) ..
		dependencies_training_weapons(building_description, "and",
			{"spear", "spear_advanced", "spear_heavy", "spear_war"}, "empire_weaponsmithy") ..

		rt(h3(_"Health Training:")) ..
		dependencies_training_food({{"fish", "meat"}, {"bread_empire"}}) ..
		dependencies_training_weapons(building_description, "and",
			{"armor_helmet", "armor", "armor_chain", "armor_gilded"}, "empire_armorsmithy") ..

		--Workers Section
		building_help_crew_string(building_description) ..

		--Building Section
		building_help_building_section(building_description) ..

		--Production Section
		building_help_production_section()
	end
}
