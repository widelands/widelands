function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "You soldiers think a good sword is everything, but where would you be if you had no garments?")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A seamstress scolding a soldier for disrespecting her profession")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Sews fur garments out of reindeer fur.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The sewing room needs %s on average to produce one fur garment."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
end
