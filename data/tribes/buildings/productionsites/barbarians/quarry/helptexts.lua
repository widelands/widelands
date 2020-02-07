function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘We open up roads and make houses from mountains.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Slogan of the stonemasons’ guild")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Cuts raw pieces of granite out of rocks in the vicinity.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "The quarry needs rocks to cut within the work area.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "The stonemason pauses %s before going back to work again."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
end
