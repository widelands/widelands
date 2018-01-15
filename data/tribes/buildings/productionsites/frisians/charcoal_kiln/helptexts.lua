-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "There is no malice in this burning coal;/The breath of heaven hath blown his spirit out,/And strew'd repentant ashes on his head.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Arthur in King John")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Produces coal out of logs.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return pgettext("frisians_building", "The charcoal kiln needs %s to produce one coal."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
end
