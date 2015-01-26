-- The Imperial Training Camp
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
   func = function(building_description)
	-- need to get this again, so the building description will be of type "trainingsite"
	local building_description = wl.Game():get_building_description(building_description.name)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("empire", building_description,
		_"Trains soldiers in ‘Attack’ and in ‘Health’."
			.. " " .."Equips the soldiers with all necessary weapons and armor parts.",
		_"Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.") ..

	--Dependencies
	-- We would need to parse the production programs to automate the parameters here; so we do it manually
	dependencies_training("empire", building_description,
		"untrained", "fulltrained-evade",
		"untrained+evade", "fulltrained") ..

	rt(h3(_"Attack Training:")) ..
	dependencies_training_food("empire", { {"fish", "meat"}, {"bread_empire"}}) ..
	dependencies_training_weapons("empire", building_description, "and",
		{"spear", "spear_advanced", "spear_heavy", "spear_war"}, "empire_weaponsmithy") ..

	rt(h3(_"Health Training:")) ..
	dependencies_training_food("empire", { {"fish", "meat"}, {"bread_empire"}}) ..
	dependencies_training_weapons("empire", building_description, "and",
		{"armor_helmet", "armor", "armor_chain", "armor_gilded"}, "empire_armorsmithy") ..

	--Workers Section
	building_help_crew_string("empire", building_description) ..

	--Building Section
	building_help_building_section("empire", building_description) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
   end
}
