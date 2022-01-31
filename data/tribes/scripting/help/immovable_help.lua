-- RST
-- immovable_help.lua
-- ------------------
--
-- This script returns a formatted entry for the in-game immovable help (Tribal Encyclopedia).
-- Pass the internal tribe name and immovable name to the coroutine to select the
-- immovable type.

include "tribes/scripting/help/format_help.lua"

--  =======================================================
--  ************ Main immovable help functions ************
--  =======================================================

-- RST
-- .. function:: immovable_help_string(tribe, immovable_description)
--
--    Displays the immovable with a helptext.
--
--    :arg tribe: the tribe that we are displaying this help for; the same immovable
--       can be used by various tribes in different ways.
--    :type tribe: :class:`LuaTribeDescription`
--
--    :arg immovable_description: the immovable_description that the help is
--       being displayed for.
--    :type immovable_description: :class:`LuaImmovableDescription`
--
--    :returns: Help string for the immovable
--
function immovable_help_string(tribe, immovable_description)
   local helptexts = immovable_description:helptexts(tribe.name)
   local result = ""
   if helptexts["purpose"] ~= nil then
      result = h2(_("Purpose")) ..
         li_object(immovable_description.name, helptexts["purpose"])
   else
      result = img(immovable_description.icon_name)
   end

   -- Build cost
   local buildcost = ""
   for ware, amount in pairs(immovable_description.buildcost) do
      local ware_description = wl.Game():get_ware_description(ware)
      buildcost = buildcost .. help_ware_amount_line(ware_description, amount)
   end

   -- Space required
   local space_required = plot_size_line(immovable_description.size)

   if (buildcost ~= "" or space_required ~= "") then
      result = result .. h2(_("Requirements"))
      if (buildcost ~= "") then
         result = result .. h3(_("Build cost:")) .. buildcost
      end
      result = result .. space_required
   end

   local becomes_list = immovable_description.becomes
   if (#becomes_list > 0) then
      result = result .. h2(_("Becomes"))
      for index, target in ipairs(becomes_list) do
         local target_description = nil
         if (wl.Game():immovable_exists(target)) then
            -- We turn into another immovable
            target_description = wl.Game():get_immovable_description(target)
         else
            -- Target must be a ship
            target_description = wl.Game():get_ship_description(target)
         end
         if (target_description ~= nil) then
            result = result .. li_image(target_description.icon_name, target_description.descname)
         end
      end
   end

   -- Terrain affinity
   local affinity = immovable_description.terrain_affinity
   if (affinity ~= nil) then
      include "scripting/help.lua"
      result = result .. h2(_("Preferred Terrains"))
      result = result .. terrain_affinity_help(immovable_description)
   end

   return result
end


return {
   func = function(tribename, immovablename)
      push_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      local immovable_description = wl.Game():get_immovable_description(immovablename)
      local r = {
         title = immovable_description.descname,
         text = immovable_help_string(tribe, immovable_description)
      }
      pop_textdomain()
      return r
   end
}
