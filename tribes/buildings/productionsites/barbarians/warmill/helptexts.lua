-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", [[An old Barbarian proverb<br> meaning that you need to take some risks sometimes.]])
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "The war mill produces all the axes that the Barbarians use for warfare.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "The Barbarian war mill is their most advanced production site for weapons. As such it needs to be upgraded from an ax workshop.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return
      pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce each type of ax in about %s on average."):bformat(ngettext("%d second", "%d seconds", 57):bformat(57))
      .. " " ..
      pgettext("barbarians_building", "All weapons require the same time for making, but the consumption of the same raw materials increases from step to step. The last two need gold.")
end