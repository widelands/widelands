function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "No frost, no heat; no rain, no draught; no rats, no locusts; naught can destroy my harvest.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A farmer's reply when asked by his chieftain why he plants such a slow-growing grain.")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Sows and harvests barley.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return "Needs free space around to plant seeds."
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The farms needs approximately %s to produce one sheath of barley."):bformat(ngettext("%d minute", "%d minutes", 5):bformat(5))
end
