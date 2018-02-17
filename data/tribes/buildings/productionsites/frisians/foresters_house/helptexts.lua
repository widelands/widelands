function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "What can ever be more beautiful than the brilliant sun’s beams shining through the glistering canopy of leaves?")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A forester explaining his choice of profession")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Plants trees in the surrounding area.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "The forester’s house needs free space within the work area to plant the trees.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The forester pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 12):bformat(12))
end
