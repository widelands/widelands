function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Whilst you have fed upon my signories,/Dispark'd my parks and fell'd my forest woods…")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Henry in Richard II")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Fells trees in the surrounding area and processes them into logs.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "The woodcutter´s house needs trees to fell within the work area.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The woodcutter pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
end
