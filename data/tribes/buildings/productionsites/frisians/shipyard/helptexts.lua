-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building. "Yard" is a wordplay with the measurement unit
   return pgettext("frisians_building", "This house may be called a shipyard, but my ships are rather longer than one yard!")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("frisians_building", "A shipwright who only constructed toy ships after being chid that his ships were too small")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
