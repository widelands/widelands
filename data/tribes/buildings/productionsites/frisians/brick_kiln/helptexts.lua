function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "If there is not enough coal, only the most foolish of leaders would deprive his brick kilns of it first.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A brickmaker arguing with his chieftain who was doing just that")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Burns bricks using granite and clay, and coal as fuel. Bricks are the most important building material.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The brick kiln needs %s on average to produce three bricks."):bformat(ngettext("%d second", "%d seconds", 84):bformat(84))
end
