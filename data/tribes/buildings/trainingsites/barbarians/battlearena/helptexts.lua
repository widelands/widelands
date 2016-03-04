function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘No better friend you have in battle than the enemy’s blow that misses.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Said to originate from Neidhardt, the famous trainer.")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return
      pgettext("barbarians_building", "Trains soldiers in ‘Evade’.")
      .. " " ..
      pgettext("barbarians_building", "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("barbarians_building", "Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If all needed wares are delivered in time, a battle arena can train evade for one soldier from 0 to the highest level in %1$s and %2$s on average."):bformat(ngettext("%d minute", "%d minutes", 1):bformat(1), ngettext("%d second", "%d seconds", 10):bformat(10))
end
