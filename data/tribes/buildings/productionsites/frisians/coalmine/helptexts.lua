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
   return pgettext("building", "Digs coal out of the ground in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal."):bformat("1/2")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The coal mine needs %s to produce two pieces of coal."):bformat(ngettext("%d second", "%d seconds", 85):bformat(85))
end
