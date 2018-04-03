function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building. Reference to frequent discussions about good scouting algorithms
   return pgettext("frisians_building", "Everyone has their own ideas how exactly we should explore the enemy’s territory… One more „improvement“ suggestion and we’ll demand meals instead of rations!")
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("frisians_building", "The desired behavior of scouts is often discussed, but nobody actually asked them for their opinion…")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Explores unknown territory.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return pgettext("frisians_building", "The scout pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
end
