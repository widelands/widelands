-- The Barbarian Reed Yard

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘We grow roofs’]],_[[Slogan of the Guild of Gardeners]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "thatchreed",
		_"The Reed Yard cultivates reed that serves two different purposes for the Barbarian tribe.",
		"Thatch reed is the traditional material for roofing, and it is woven into the extremely durable cloth that they use for their ships’ sails.",
		"1") ..

	--Dependencies

	building_help_outputs("barbarians", building_description, {"constructionsite"}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"gardener"}, "shovel") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"A reed yard can produce a sheaf of thatch reed in about %s on average.":bformat("65s"))
   end
}
