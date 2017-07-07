function building_helptext_lore()
   -- TRANSLATORS#: Lore helptext for a building
   return pgettext("frisians_building", "")
end

function building_helptext_lore_author()
   -- TRANSLATORS#: Lore author helptext for a building
   return ""
end

function building_helptext_purpose()
   -- TRANSLATORS#: Purpose helptext for a building
   return pgettext("building", "Digs granite out of the ground in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any granite."):bformat("1/2")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "Calculation needed")
end
