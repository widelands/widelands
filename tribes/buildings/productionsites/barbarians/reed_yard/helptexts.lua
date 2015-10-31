function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘We grow roofs’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Slogan of the Guild of Gardeners")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "The reed yard cultivates reed that serves two different purposes for the Barbarian tribe.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "Thatch reed is the traditional material for roofing, and it is woven into the extremely durable cloth that they use for their ships’ sails.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "A reed yard can produce a sheaf of thatch reed in about %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
end