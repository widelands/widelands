-- RST
-- help.lua
-- --------
--
-- This script contains functions that are used both in the Tribal Encyclopedia
-- and the Editor help.

include "scripting/richtext.lua"

-- RST
-- .. function:: terrain_affinity_list(immovable_description)
--
--    Returns list of terrains that the given
--    immovable is most likely to grow on, sorted in descending order by probability.
--
--    :arg immovable_description: The immovable type that we want the information for.
--    :type immovable_description: :class:`LuaImmovableDescription`
--    :returns: list of :class:`LuaTerrainDescription` and probabilities.
--
function terrain_affinity_list(immovable_description)
   local world = wl.World();
   local terrain_list = {}
   for i, terrain in ipairs(world.terrain_descriptions) do
      local probability = immovable_description:probability_to_grow(terrain)
      if (probability > 0.01) then
         -- sort the terrains by percentage
         i = 1
         while (terrain_list[i] and (terrain_list[i].probability > probability)) do
            i = i + 1
         end

         for j = #terrain_list, i, -1 do
            terrain_list[j+1] = terrain_list[j]
         end
         terrain_list[i] = {terrain = terrain, probability = probability}
      end
   end
   return terrain_list
end


-- RST
-- .. function:: terrain_affinity_help(immovable_description)
--
--    Returns a formatted help string for the terrains that the given
--    immovable is most likely to grow on.
--
--    :arg immovable_description: The immovable type that we want the information for.
--    :type immovable_description: :class:`LuaImmovableDescription`
--    :returns: a richtext-formatted list of terrain images, terrain names and probabilities.
--
function terrain_affinity_help(immovable_description)
   push_textdomain("widelands")
   local world = wl.World();
   local result = ""
   local terrain_list = terrain_affinity_list(immovable_description)

   for k,v in ipairs(terrain_list) do
      if (k <= 10 or v.probability > 0.25) then
         result = result .. li_image(v.terrain.representative_image,
               -- TRANSLATORS: Terrain name (Climate)
               v.terrain.descname .. "<br>" ..
               -- TRANSLATORS: Help text - Probability to grow for an immovable
               (_("%2.1f%%")):bformat(100 * v.probability)
            ) .. vspace(6)
      end
   end
   pop_textdomain()
   return result
end
