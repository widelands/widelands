function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("frisians_building", "You want to know why I don’t get more meat from oxen than from bunnies? Of course I do! But I like to eat up the surplus myself.")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("frisians_building", "Hunters don’t consider it illogical that all kinds of animals produce the same amount of meat")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Hunts animals to produce meat.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "The hunter’s house needs animals to hunt within the work area.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The hunter pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 35):bformat(35))
end
