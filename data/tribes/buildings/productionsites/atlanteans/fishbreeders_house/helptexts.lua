-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext("atlanteans_building", [[‘Only after the last duck has been shot down<br>]] ..
            [[Only after the last deer has been put to death<br>]] ..
            [[Only after the last fish has been caught<br>]] ..
            [[Then will you find that spiders are not to be eaten.’]])
end

function building_helptext_lore_author()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext("atlanteans_building", "Prophecy of the fish breeders")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Breeds fish.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return no_performance_text_yet()
end
