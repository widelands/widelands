-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "")
end

function building_helptext_purpose()
   -- #TRANSLATORS: Purpose helptext for a building
   return pgettext("frisians_building", "Equips recruits and trains them as soldiers.")
end

function building_helptext_note()
   -- #TRANSLATORS: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- #TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The barracks needs %s to recruit one soldier."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
end
