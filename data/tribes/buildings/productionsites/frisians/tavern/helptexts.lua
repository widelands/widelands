function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Nothing but fruit all day… Couldn’t you hurry up a bit?")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Hungry customers in times of a shortage of smoked fish and meat")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Prepares rations to feed the scouts and miners.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The tavern can produce one ration in %s on average if the supply is steady; otherwise, it will take 50%% longer."):bformat(ngettext("%d second", "%d seconds", 33):bformat(33))
end
