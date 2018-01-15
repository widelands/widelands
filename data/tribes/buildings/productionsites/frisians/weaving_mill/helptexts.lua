-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "The spinsters and the knitters in the sun,/And the free maids that weave their thread with bones,/Do use to chaunt it.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Duke in Twelfth Night or What You Will")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Sews cloth for ship sails out of reed and reindeer fur.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return pgettext("frisians_building", "The weaving mill needs %s to produce one piece of cloth."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
end
