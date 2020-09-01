function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "If my honey tastes bitter, I must have left some bee stings in it. There are never any bee stings in my honey, therefore, it is not bitter.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A beekeeper ignoring a customer’s complaint")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("frisians_building", "Keeps bees and lets them swarm over flowering fields to produce honey.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "Needs medium-sized fields (barley, wheat, reed, corn or blackroot) or bushes (berry bushes or grapevines) nearby.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The beekeeper pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
end
