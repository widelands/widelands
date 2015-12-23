function building_helptext_lore()
   -- TRANSLATORS#: Lore helptext for a building
   return pgettext("barbarians_building", [[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]])
end

function building_helptext_lore_author()
   -- TRANSLATORS#: Lore author helptext for a building
   return ""
end

function building_helptext_purpose()
   -- TRANSLATORS#: Purpose helptext for a building
   return pgettext("building", "Digs coal out of the ground in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more coal."):bformat("10%")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If the food supply is steady, this mine can produce coal in %s on average."):bformat(ngettext("%d second", "%d seconds", 14.4):bformat(14.4))
end
