-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS#: Lore helptext for a building
   return no_lore_text_yet()
end

function building_helptext_lore_author()
   -- TRANSLATORS#: Lore author helptext for a building
   return no_lore_author_text_yet()
end

function building_helptext_purpose()
   push_textdomain("scenario_emp04.wmf")
   -- TRANSLATORS: Purpose helptext for a building
   local r = pgettext("building", "In the temple of Vesta, the wares to worship are stored. It is the home of the priestesses and the guards of the goddess.")
   pop_textdomain()
   return r
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return ""
end
