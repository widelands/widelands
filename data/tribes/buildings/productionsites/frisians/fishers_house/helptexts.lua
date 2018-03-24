function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Hunters can’t sell anything on a Friday, but fishers don’t have such problems.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A hunter admiring a fisher")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("frisians_building", "Fishes on the coast near the fisher’s house.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "The fisher’s house needs water full of fish within the work area.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The fisher pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 16):bformat(16))
end
