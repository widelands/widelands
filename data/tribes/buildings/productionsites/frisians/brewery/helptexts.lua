function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "I know no single master miner who’ll ever work without a nice pint of beer!")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A brewer boasting about the importance of his profession")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Brews beer for miners and soldier training.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The brewery needs %s on average to brew one mug of beer."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
end
