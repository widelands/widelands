-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "The carried me to the tavern and made me drunk, and afterward pick'd my pocket.")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Slender in The Merry Wives of Windsor")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Prepares rations to feed the scouts and miners.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return pgettext("frisians_building", "The tavern can produce one ration in %s on average if the supply is steady; otherwise, it takes 50 percent longer."):bformat(ngettext("%d second", "%d seconds", 33):bformat(33))
end
