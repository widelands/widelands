function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "My bushes may not be as tall as your trees, but I don’t know anybody who likes to eat bark!")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A berry farmer to a forester")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Plants berry bushes in the surrounding area.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "The berry farm needs free space within the work area to plant the bushes.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The berry farmer pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 21):bformat(21))
end
