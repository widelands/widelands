function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "I'll make thee eat iron like an ostridge, and swallow my sword like a great pin, ere thou and I part.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Cade in Henry VI/2")
end

function building_helptext_purpose()
   -- TRANSLATORS#: Purpose helptext for a building
   return pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any iron ore."):bformat("1/2")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The iron mine needs %s to produce one piece of iron ore."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
end
