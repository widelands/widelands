-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", [[‘Then he hit the door with his ax and, behold,<br>]] ..
            [[the door only trembled, but the shaft of the ax burst into a thousand pieces.’]])
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", [[Legend about a quarrel between the brothers Harthor and Manthor,<br>]] ..
            [[Manthor is said to be the inventor of blackwood]])
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Hardens logs by fire into blackwood, which is used to construct robust buildings.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
