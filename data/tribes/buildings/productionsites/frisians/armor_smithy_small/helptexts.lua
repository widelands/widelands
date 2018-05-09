-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("frisians_building", "I don’t forge swords because soldiers need ’em, but soldiers get ’em because I forge ’em.")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("frisians_building", "A blacksmith pointing out his influence on soldier training")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Produces basic weapons and helmets for the soldiers.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
