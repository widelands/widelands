function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("frisians_building", "")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("frisians_building", "")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return
      pgettext("frisians_building", "Trains soldiers in Attack up to level 3 as well as in Defence and Health to level 1.")
      .. " " ..
      pgettext("frisians_building", "Equips the soldiers with all necessary weapons and armour parts.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "Frisian soldiers cannot be trained in Evade and will remain at their initial level.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building. Placeholders are for "X minutes" and "Y seconds"
   return pgettext("frisians_building", "Calculation needed")
end
