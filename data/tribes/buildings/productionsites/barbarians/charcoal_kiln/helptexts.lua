-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", [[‘What’s the work of a charcoal burner?<br>]] ..
         [[He’s the tribe’s most plodding earner!<br>]] ..
         [[Logs on logs he piles up high,<br>]] ..
         [[Until the kiln will reach the sky.<br>]] ..
         [[He sets the fire, sees it smolder<br>]] ..
         [[The logs he carried on his shoulder.<br>]] ..
         [[Burnt down to just one single coal<br>]] ..
         [[This troubles much a burner’s soul.’]])
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Song of the charcoal burners")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Burns logs into charcoal.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
