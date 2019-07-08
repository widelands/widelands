function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘The smart leader builds roads, while the really wise leader breeds cattle.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Khantarakh, ‘The Modern Barbarian Economy’,<br> 5ᵗʰ cowhide ‘Traffic and Logistics’")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Breeds strong oxen for adding them to the transportation system.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce an ox in %s on average."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
end
