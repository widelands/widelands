-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "thy rich leas/Of wheat, rye, barley, fetches, oats, and pease;/Thy turfy mountains, where live nibbling sheep,/And flat meads thatch'd with stover, them to keep")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Iris in The Tempest")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Sows and harvests barley.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The farms needs approximately %s to produce one sheath of barley."):bformat(ngettext("%d minute", "%d minutes", 5):bformat(5))
end
