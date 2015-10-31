function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘What do you mean, my fish ain’t fresh?!’")
end

function building_helptext_lore_author()
   -- #TRANSLATORS: Lore author helptext for a building
   return ""
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Fishes on the coast near the hut.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "The fisher’s hut needs water full of fish within the work area.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "The fisher pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 18):bformat(18))
end