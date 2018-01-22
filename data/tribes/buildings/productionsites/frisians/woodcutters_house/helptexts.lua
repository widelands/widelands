function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "I cannot see a tree without imagining what it would look like in terms of furniture.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Over-enthusiastic woodcutter")
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
