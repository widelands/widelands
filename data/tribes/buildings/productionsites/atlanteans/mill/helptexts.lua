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
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("atlanteans_building", "Grinds blackroots and corn to produce blackroot flour and cornmeal, respectively.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return pgettext("atlanteans_building", "When no cornmeal is required, the mill will try to produce blackroot flour even when there is no demand for it.")
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
