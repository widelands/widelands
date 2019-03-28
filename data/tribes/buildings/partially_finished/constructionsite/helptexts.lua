-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS#: Lore helptext for a building
   return pgettext("barbarians_building", "‘Don't swear at the builder who is short of building materials.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS#: Lore author helptext for a building
   return pgettext("barbarians_building", "Barbarian Proverb,<br> widely used for under equipped tasks as well")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "A new building is being built at this construction site.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
