-- The Imperial Gold Mine

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_empire")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("empire", building_description,
		_"Digs gold ore out of the ground in mountain terrain.") ..
-- TODO(GunChleoc) calculation needed "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any gold ore.":bformat("1/3"))

	--Dependencies
	building_help_dependencies_production("empire", building_description) ..

	--Workers Section
	building_help_crew_string("empire", building_description) ..

	--Building Section
	building_help_building_section("empire", building_description) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
  end
}
