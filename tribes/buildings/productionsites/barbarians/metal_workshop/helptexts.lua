function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘We make it work!’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Inscription on the threshold of the now ruined Olde Forge at Harradsheim, the eldest known smithy.")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Forges all the tools that your workers need.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "The Barbarian metal workshop is the basic production site in a series of three buildings and creates all the tools that Barbarians need. The others are for weapons.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce each tool in about %s on average."):bformat(ngettext("%d second", "%d seconds", 67):bformat(67))
end
