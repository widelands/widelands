function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("barbarians_building", "‘He who has enough bread will never be too tired to dig the ore and wield the ax.’")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("barbarians_building", "Khantarakh, ‘The Modern Barbarian Economy’,<br>3ʳᵈ cowhide ‘Craftsmanship and Trade’")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("barbarians_building", "Bakes pitta bread for soldiers and miners alike.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce a pitta bread in %s on average."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
end
