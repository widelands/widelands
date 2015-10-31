function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘Forming new stone from old with fire and water.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Ragnald the Child’s answer to the question, what he’s doing in the dirt. His ‘new stone’ now is an important building material.")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "The lime kiln processes granite to make ‘grout’, a substance that solidifies and so reinforces masonry.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "The lime kiln’s output will only go to construction sites that need it. Those are predominantly houses that work with fire, and some military sites.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce grout in about %s on average."):bformat(ngettext("%d second", "%d seconds", 41):bformat(41))
end