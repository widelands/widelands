-- The Imperial Arena

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_empire")

return {
   func = function(building_description)
	-- need to get this again, so the building description will be of type "trainingsite"
	local building_description = wl.Game():get_building_description("empire", building_description.name)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]]) ..

	--General Section
	building_help_general_string("empire", building_description, "soldier",
		_"Trains soldiers in ‘Evade’." .. " " .. _"‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.",
		_"Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.") ..

	--Dependencies
	-- We would need to parse the production programs to automate the parameters here; so we do it manually
	dependencies_training("empire", building_description,
		"untrained", "untrained+evade1",
		"fulltrained-evade", "fulltrained-evade1") ..

	rt(h3(_"Evade Training:")) ..
	dependencies_training_food("empire", { {"fish", "meat"}, {"bread"}}) ..

	--Workers Section
	building_help_crew_string("empire", building_description) ..

	--Building Section
	building_help_building_section("empire", building_description) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
   end
}
