-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Why on earth shouldn’t we be able to catch fish even in the desert?")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "The fisherman who invented aqua farming")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Breeds fish as food for soldiers and miners.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("building", "The aqua farm needs holes in the ground that were dug by a clay pit’s worker nearby to use as fishing ponds.")
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
