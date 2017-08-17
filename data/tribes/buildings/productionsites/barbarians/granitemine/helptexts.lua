function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘I can handle tons of granite, man, but no more of your vain prattle.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", [[This phrase was the reply Rimbert the miner – later known as Rimbert the loner – gave, when he was asked to remain seated on an emergency meeting at Stonford in the year of the great flood. <br> The same man had all the 244 granite blocks ready only a week later, and they still fortify the city’s levee.]])
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Carves granite out of the rock in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return
      (pgettext("barbarians_building","This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more granite."):bformat("5%") .. "<br>"
         -- TRANSLATORS: 'It' is a mine
         .. pgettext("barbarians_building", "It cannot be enhanced."))
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If the food supply is steady, this mine can produce granite in %s on average."):bformat(ngettext("%d second", "%d seconds", 20):bformat(20))
end
