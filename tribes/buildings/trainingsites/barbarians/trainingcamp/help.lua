-- The Barbarian Training Camp
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
		dependencies_training("barbarians", building_description,
			"untrained", "fulltrained-evade", "untrained+evade", "fulltrained") ..

		rt(h3(_"Attack Training:")) ..
		dependencies_training_food({{"fish", "meat"}, {"bread_barbarians"}}) ..
		dependencies_training_weapons(building_description, "and",
			{"ax_sharp", "ax_broad"}, "axfactory") ..
		dependencies_training_weapons(building_description, "or",
			{"ax_sharp", "ax_broad", "ax_bronze", "ax_battle", "ax_warriors"}, "barbarians_warmill") ..

		rt(h3(_"Health Training:")) ..
		dependencies_training_food({{"fish", "meat"}, {"bread_barbarians"}}) ..
		dependencies_training_weapons(building_description, "and",
			{"helmet", "helmet_mask", "helmet_warhelm"}, "barbarians_helmsmithy") ..

		--Workers Section
		building_help_crew_string(building_description) ..

		--Building Section
		building_help_building_section(building_description) ..

		--Production Section
		building_help_production_section()
	end
}
