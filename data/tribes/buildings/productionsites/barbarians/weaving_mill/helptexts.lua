-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", [[‘Fine cloth and jewelry mean nothing to a true Barbarian.<br>]] ..
            [[But a noble sail in the wind will please his eyes.’]])
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Khantarakh, ‘The Modern Barbarian Economy’,<br>9ᵗʰ cowhide ‘Seafaring and Conquest’")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Weaves cloth out of reed.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
