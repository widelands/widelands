-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Who says a beast of burden cannot be useful for other things than transport?")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "The reindeer breeder who first proposed using reindeer fur for clothing")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("frisians_building", "Breeds strong reindeer for adding them to the transportation system. Also keeps them for their fur, which is turned into armor and cloth.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext ("frisians_building", "If the supply is steady, the reindeer farm produces one meat after producing three pieces of fur.")
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet ()
end
