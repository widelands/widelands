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
   return pgettext("atlanteans_building", "Fishes on the coast near the house.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("atlanteans_building", "The fisher’s house needs water full of fish within the work area.") .. " " .. pgettext("atlanteans_building", "Build a fisher breeder’s house close to the fisher’s house to make sure that you don’t run out of fish.")
end

function building_helptext_performance()
   -- #TRANSLATORS: Performance helptext for a building
   return no_performance_text_yet()
end
