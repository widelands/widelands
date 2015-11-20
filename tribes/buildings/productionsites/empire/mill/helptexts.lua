-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- #TRANSLATORS: Lore helptext for a building
   return no_lore_text_yet()
end

function building_helptext_lore_author()
   -- #TRANSLATORS: Lore author helptext for a building
   return no_lore_author_text_yet()
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Grinds wheat to produce flour.")
end

function building_helptext_note()
   -- #TRANSLATORS: Note helptext for a building
   return ""
end

function building_helptext_performance()
	-- TRANSLATORS: Performance helptext for a building
	return pgettext("empire_building", "The miller needs 19 seconds on average to grind wheat into a sack of corn.")
end
