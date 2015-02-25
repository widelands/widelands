-- The Barbarian Battle Arena"

include "data/scripting/formatting.lua"
include "data/tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	-- need to get this again, so the building description will be of type "trainingsite"
	local building_description = wl.Game():get_building_description("barbarians", building_description.name)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘No better friend you have in battle than the enemy’s blow that misses.’]], _[[Said to originate from Neidhardt, the famous trainer.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"Trains soldiers in ‘Evade’." .. " " .. _"‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.",
		_"Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level.") ..

	--Dependencies
	-- We would need to parse the production programs to automate the parameters here; so we do it manually
	dependencies_training("barbarians", building_description,
		"untrained", "untrained+evade",
		"fulltrained-evade", "fulltrained") ..

	rt(h3(_"Evade Training:")) ..
	dependencies_training_food("barbarians", { {"fish", "meat"}, {"strongbeer"}, {"pittabread"}}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"If all needed wares are delivered in time, a battle arena can train evade for one soldier from 0 to the highest level in %1$s and %2$s on average.":bformat(
		ngettext("%d minute", "%d minutes", 1):bformat(1),
		ngettext("%d second", "%d seconds", 10):bformat(10)
	))
   end
}
