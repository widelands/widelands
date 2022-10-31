-- RST
-- help.lua
-- --------
--
-- This script contains functions that are used both in the Tribal Encyclopedia
-- and the Editor help. To get descriptions see: :attr:`wl.Descriptions`.
--
-- .. code-block:: lua
--
--    include "scripting/help.lua"
--

include "scripting/richtext.lua"


-- RST
-- .. function:: linkify_encyclopedia_object(descr)
--
--    Return the localized name of the given unit description
--    as a hyperlink pointing to its encyclopedia entry.
--
--    :arg descr: The unit description to linkify.
--    :returns: Richtext markup for the hyperlink.
function linkify_encyclopedia_object(descr)
   return a(descr.species ~= "" and descr.species or descr.descname, "ui", "encyclopedia", descr.name)
end

-- RST
-- .. function:: tree_affinity_list(terrain_description)
--
--    Returns list of trees that is most likely to grow on the given
--    terrain, sorted in descending order by probability.
--
--    :arg terrain_description: The terrain type that we want the information for.
--    :type terrain_description: :class:`~wl.map.TerrainDescription`
--    :returns: list of :class:`~wl.map.ImmovableDescription` that have the attribute "tree" and probabilities.
--
function tree_affinity_list(terrain_description)
   local tree_list = {}
   for i, immovable in ipairs(wl.Descriptions().immovable_descriptions) do
      if (immovable:has_attribute("tree")) then
         local probability = immovable:probability_to_grow(terrain_description)
         if (probability > 0.01) then
            -- sort the trees by percentage
            i = 1
            while (tree_list[i] and (tree_list[i].probability > probability)) do
               i = i + 1
            end

            for j = #tree_list, i, -1 do
               tree_list[j+1] = tree_list[j]
            end
            tree_list[i] = {tree = immovable, probability = probability}
         end
      end
   end
   return tree_list
end

-- RST
-- .. function:: terrain_affinity_list(immovable_description)
--
--    Returns list of terrains that the given
--    immovable is most likely to grow on, sorted in descending order by probability.
--
--    :arg immovable_description: The immovable type that we want the information for.
--    :type immovable_description: :class:`~wl.map.ImmovableDescription`
--    :returns: list of :class:`~wl.map.TerrainDescription` and probabilities.
--
function terrain_affinity_list(immovable_description)
   local terrain_list = {}
   for i, terrain in ipairs(wl.Descriptions().terrain_descriptions) do
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
--    :type immovable_description: :class:`~wl.map.ImmovableDescription`
--    :returns: a richtext-formatted list of terrain images, terrain names and probabilities.
--
function terrain_affinity_help(immovable_description)
   push_textdomain("widelands")
   local result = ""
   local terrain_list = terrain_affinity_list(immovable_description)

   for k,v in ipairs(terrain_list) do
      if (k <= 10 or v.probability > 0.25) then
         result = result .. li_image(v.terrain.representative_image,
               (wl.Editor ~= nil and linkify_encyclopedia_object(v.terrain) or v.terrain.descname) .. "<br>" ..
               -- TRANSLATORS: Help text - Probability to grow for an immovable
               (_("%2.1f%%")):bformat(100 * v.probability)
            ) .. vspace(6)
      end
   end
   pop_textdomain()
   return result
end
