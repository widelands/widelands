-- RST
-- objective_utils.lua
-- ---------------------
--
-- This script contains utility functions for typical tasks that need to
-- be checked for objectives.

-- =======================================================================
--                             PRIVATE FUNCTIONS
-- =======================================================================
function _check_for_region(plr, which, region)
   local carr = {}
   for idx,f in ipairs(region) do
      if f.immovable and f.immovable.owner == plr then
         if carr[f.immovable.descr.name] == nil then
            carr[f.immovable.descr.name] = 1
         else
            carr[f.immovable.descr.name] = carr[f.immovable.descr.name] + 1
         end
      end
   end
   for house,count in pairs(which) do
      if carr[house] == nil or carr[house] < count then
         return false
      end
   end
   return true
end


-- RST
-- .. function:: check_for_buildings(plr, which[, region])
--
--    Checks if the number of buildings defined in which are found for the
--    given player. If region is given, buildings are only searched on the
--    corresponding fields. If more buildings or equal the number of requested
--    buildings are found, this function returns true.
--
--    Example usage:
--
--    .. code-block:: lua
--
--       check_for_buildings(wl.Game().players[1], {lumberjacks_hut=2, quarry=1})
--
--    :arg plr: Player to check for
--    :type plr: :class:`wl.game.Player`
--    :arg region: array of fields to check for the buildings
--    :type region: :class:`array` of :class:`wl.map.Field`
--    :arg which: (name,count) pairs for buildings to check for.
--    :type which: :class:`table`
--
--    :returns: true if the requested buildings were found, false otherwise
function check_for_buildings(plr, which, region)
   if region then return _check_for_region(plr, which, region) end

   for house, count in pairs(which) do
      if #plr:get_buildings(house) < count then return false end
   end
   return true
end