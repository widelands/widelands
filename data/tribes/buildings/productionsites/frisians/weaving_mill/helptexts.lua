function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("frisians_building", "Reindeer’s fur and roofing reed/ These items two make up the seed/ For ships of wood to chain the gales/ In sturdy, beautious, blowing sails!")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("frisians_building", "A seamstress’s work song")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Sews cloth for ship sails out of reed and reindeer fur.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return pgettext("frisians_building", "The weaving mill needs %s to produce one piece of cloth."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
end
