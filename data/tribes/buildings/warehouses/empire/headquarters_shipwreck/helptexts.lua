-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS#: Lore helptext for a building
   return no_lore_text_yet()
end

function building_helptext_lore_author()
   -- TRANSLATORS#: Lore author helptext for a building
   return no_lore_author_text_yet()
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("empire_building", "Although this ship ran aground, it still serves as accommodation for your people. It also stores your wares and tools.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return
      pgettext("empire_building", "The headquarters shipwreck is your main building.")
      -- .. "<br>" .. no_lore_text_yet()
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return ""
end
