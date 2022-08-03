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
   local image = immovable_description.icon_name
   if helptexts.purpose ~= nil then
      result = h2(_("Purpose")) ..
         li_object(immovable_description.name, helptexts.purpose)
   elseif image ~= "" then
      result = p(vspace(14) .. img(immovable_description.icon_name))
   end

   if helptexts.note ~= nil then
      result = result .. h2(_("Note")) .. p(helptexts.note)
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
      if (buildcost ~= "") then
         result = result .. h2(_("Requirements"))
         result = result .. h3(_("Build cost:")) .. buildcost
         result = result .. plot_size_line(immovable_description.size)
      else
         result = result .. h2(_("Size"))
         result = result .. plot_size_line(immovable_description.size, true)
      end
      if (immovable_description.size == "small") then
         result = result .. p(_("Workers and animals can walk across fields with this immovable."))
      else
         result = result .. p(_("Workers and animals can’t walk across fields with this immovable."))
      end
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
            local icon = target_description.icon_name
            if (icon ~= "") then
               result = result ..
                  li_image(icon, target_description.descname)
            else
               result = result .. li(target_description.descname)
            end
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
      local t = immovable_description.descname
      if immovable_description:has_attribute("tree") then
         t = immovable_description.species
      end
      local r = {
         title = t,
         text = immovable_help_string(tribe, immovable_description)
      }
      pop_textdomain()
      return r
   end
}
