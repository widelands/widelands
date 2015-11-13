function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘Let the first one drive away the hunger, the second one put you at ease; when you have swallowed up your third one, it’s time for the next shift!’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Widespread toast among miners")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Produces beer of the lower grade. This beer is a vital component of the snacks that inns and big inns prepare for miners in deep mines.")
end

function building_helptext_note()
   -- #TRANSLATORS: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce beer in about %s on average."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
end
