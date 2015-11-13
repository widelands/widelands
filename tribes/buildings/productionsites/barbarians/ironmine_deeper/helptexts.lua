function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘I look at my own pick wearing away day by day and I realize why my work is important.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Quote from an anonymous miner.")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more iron ore."):bformat("10%")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If the food supply is steady, this mine can produce iron ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 17.6):bformat(17.6))
end
