function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "No other tribe has ever mastered the art of charcoal burning as we have!")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "The inventor of the Frisian charcoal kiln")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Burns logs into charcoal.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return pgettext("building", "The charcoal burner's house needs holes in the ground that were dug by a clay pit’s worker nearby to erect charcoal stacks in them.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The charcoal burner's house needs %s on average to produce one coal."):bformat(ngettext("%d second", "%d seconds", 80):bformat(80))
end
