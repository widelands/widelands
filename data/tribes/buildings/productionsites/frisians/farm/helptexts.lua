-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "No frost, no heat; no rain, no drought; no rats, no locusts; naught can destroy my harvest.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A farmer’s reply when asked by his chieftain why he was planting such a slow-growing grain.")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Sows and harvests barley.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext ("frisians_building", "The farm needs free space within the work area to plant seeds.")
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
