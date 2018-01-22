-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Don't complain if these garments are too heavy – they're not supposed to be light but to keep you alive a bit longer!")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "A trainer scolding a soldier in training")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Equips fur clothes with iron or gold to produce good armour.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
