-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Now that you have two swords, there’s more of a risk you’ll accidentally stab yourself, but if you got this far, you’ll likely master this challenge as well.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A trainer training a soldier")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext ("frisians_building", "Trains soldiers in Attack, Defense and Health to the final level. Equips the soldiers with all necessary weapons and armor parts.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "Trains only soldiers who have been trained to the maximum level by the Training Camp.")
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building. Placeholders are for "4 minutes" and "40 seconds"
   return no_performance_text_yet ()
end
