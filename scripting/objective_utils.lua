-- RST
-- objective_utils.lua
-- ---------------------
--
-- This script contains utility functions for typical tasks that need to 
-- be checked for objectives.

-- RST
-- .. function:: check_for_buildings(region, which)
--
--    Checks if the number of buildings defined in which are found in the region
--    given. 
--
--    Example usage:
--
--    .. code-block:: lua
--
--       check_for_buildings(wl.map.Field(20,20):region(8), {lumberjacks_hut=2, quarry=1})
--
--    :arg region: array of fields to check for the buildings
--    :type region: :class:`array` of :class:`wl.map.Field`
--    :arg which: (name,count) pairs for buildings to check for.
--    :type which: :class:`table`
--
--    :returns: true if the requested buildings were found, false otherwise
function check_for_buildings(region, which)
   carr = {}
   for idx,f in ipairs(region) do
      if f.immovable then
         if carr[f.immovable.name] == nil then
            carr[f.immovable.name] = 1
         else
            carr[f.immovable.name] = carr[f.immovable.name] + 1
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



