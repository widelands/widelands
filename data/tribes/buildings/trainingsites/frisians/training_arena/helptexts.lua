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
      pgettext("frisians_building", "Trains soldiers in Attack, Defence and Health to the final level.")
      .. " " ..
      pgettext("frisians_building", "Equips the soldiers with all necessary weapons and armour parts.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "Trains only soldiers who have been trained to the maximum level by the Small Training Camp.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building. Placeholders are for "4 minutes" and "40 seconds"
   return pgettext("frisians_building", "Calculation needed")
end
