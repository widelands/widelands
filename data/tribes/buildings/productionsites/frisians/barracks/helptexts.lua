function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("frisians_building", "You have thirty seconds to learn the basics of swordfighting and how to stay alive in battle. A third of that time has gone by for the introduction alone! You’d better pay close attention to me in order to make the most of it. Now here is your new short sword, forged just for you by our best blacksmiths. Time’s up everyone, now go occupy your sentinels!")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("frisians_building", "A trainer greeting the new recruits")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("frisians_building", "Equips recruits and trains them as soldiers.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The barracks needs %s on average to recruit one soldier."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
end
