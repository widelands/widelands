-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return no_lore_text_yet()
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return no_lore_author_text_yet()
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext ("amazons_building", "Trains soldiers in Evade, Defense and Health. Equips the soldiers with all necessary equipment and armor parts.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("amazons_building", "Amazons need chocolate to train their soldiers properly.")
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building. Placeholders are for "4 minutes" and "40 seconds"
   return no_performance_text_yet ()
end
