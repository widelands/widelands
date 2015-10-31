function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘He who is strong shall neither forgive nor forget, but revenge injustice suffered – in the past and for all future.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Chief Chat’Karuth in a speech to his army.")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return
      pgettext("barbarians_building", "Trains soldiers in ‘Attack’ and in ‘Health’.")
      .. " " ..
      pgettext("barbarians_building", "Equips the soldiers with all necessary weapons and armor parts.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   -- TRANSLATORS: %1$s = X minute(s)
   -- TRANSLATORS: %2$s = X second(s)
   return pgettext("barbarians_building", "If all needed wares are delivered in time, a training camp can train one new soldier in attack and health to the final level in %1$s and %2$s on average."):bformat(ngettext("%d minute", "%d minutes", 4):bformat(4), ngettext("%d second", "%d seconds", 40):bformat(40))
end