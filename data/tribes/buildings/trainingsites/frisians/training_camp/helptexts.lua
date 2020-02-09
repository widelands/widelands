-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Just be quiet, listen carefully, and do try not to stab yourself until I’ve explained to you how to hold a broadsword.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A trainer training a soldier")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext ("frisians_building", "Trains soldiers in Attack up to level 3 as well as in Defense and Health to level 1. Equips the soldiers with all necessary weapons and armor parts.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext ("frisians_building", "Frisian soldiers cannot train in Evade and will remain at their initial level.")
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building.
   return no_performance_text_yet ()
end
