-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "I am prepar'd; here is my keen-edged sword,/Deck'd with five flower-de-luces on each side,/The which at Touraine, in Saint Katherine's church-yard,/Out of a great deal of old iron I chose forth.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Joan of Arc in Henry VI/1")
end

function building_helptext_purpose()
   -- #TRANSLATORS: Purpose helptext for a building
   return pgettext("frisians_building", "Recycles old armour and weapon parts from training sites into iron and gold.")
end

function building_helptext_note()
   -- #TRANSLATORS: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- #TRANSLATORS: Performance helptext for a building
   return no_performance_text_yet()
end
