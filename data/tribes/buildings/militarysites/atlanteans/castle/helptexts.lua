-- RST
-- tribes/buildings/militarysites/<name>/helptexts.lua
-- ---------------------------------------------------
--
-- The functions in this file return texts that are used by the Tribal Encyclopedia.
--
-- See also the helper functions in ``tribes/scripting/help/global_helptexts.lua``
--

-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

-- RST
-- .. function:: building_helptext_lore()
--
--    Returns a localized string with a lore helptext for this building.
--    If you don't have a lore text yet, use ``no_lore_text_yet()`` from the ``global_helptexts.lua`` script.
--
--    :returns: Localized lore text
function building_helptext_lore()
   -- TRANSLATORS#: Lore helptext for a building
   return no_lore_text_yet()
end

-- RST
-- .. function:: building_helptext_lore_author()
--
--    Returns a localized string with a fictional author for the lore helptext for this building. The returned string can be empty.
--    If you don't have an author yet, use ``no_lore_author_text_yet()`` from the ``global_helptexts.lua`` script.
--
--    :returns: Localized lore author or empty string.
function building_helptext_lore_author()
   -- TRANSLATORS#: Lore author helptext for a building
   return no_lore_author_text_yet()
end

-- RST
-- .. function:: building_helptext_purpose()
--
--    Returns a localized string with a purpose helptext for this building. This should be individually defined for all buildings. You can use ``no_purpose_text_yet()`` from the ``global_helptexts.lua`` script if you're not ready to define this yet.
--
--    :returns: Localized purpose text
function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Garrisons soldiers to expand your territory.")
end

-- RST
-- .. function:: building_helptext_note()
--
--    Returns a localized string with a note that is added to the purpose. This can be empty.
--
--    :returns: Localized note text or empty string.
function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
end

-- RST
-- .. function:: building_helptext_performance()
--
--    Returns a localized string with a performance helptext for this building. You can use ``no_purpose_text_yet()`` from the ``global_helptexts.lua`` script if the performance hasn't been calculated yet. Leave empty when this isn't applicable (e.g. for militarysites or warehouses).
--
--    :returns: Localized performance text or empty string.
function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return ""
end
