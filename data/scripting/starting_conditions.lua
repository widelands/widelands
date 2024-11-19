include "scripting/coroutine.lua"
include "scripting/ui.lua"

-- RST
-- starting_conditions.lua
-- -----------------------
--
-- This script contains some convenience functions intended mainly for use
-- in the Discovery and New World starting conditions.
--
-- To make these functions available include this file at the beginning
-- of a script via:
--
-- .. code-block:: lua
--
--    include "scripting/starting_condition.lua"
--

-- =======================================================================
--                             PUBLIC FUNCTIONS
-- =======================================================================

-- RST
-- .. function:: sleep_then_goto(player, sleeptime, field)
--
--    Wait for the given number of milliseconds, then center the
--    interactive player's map view on the given Field.
--
--    This function returns immediately after calling.
--
--    :arg player: The player to move the view for
--    :type player: :class:`~wl.game.Player`
--    :arg sleeptime: The number of milliseconds to wait
--    :type sleeptime: :class:`integer`
--    :arg field: The field to center the view on
--    :type field: :class:`~wl.map.Field`
--
--    :returns: :const:`nil`
function sleep_then_goto(player, sleeptime, field)
   run(function()
      sleep(sleeptime)
      if player.number == wl.Game().interactive_player then scroll_to_field(field, true) end
   end)
end

-- RST
-- .. function:: launch_expeditions(player, items)
--
--    Creates some expedition ships in random places with the
--    given additional items on them. If called for the interactive
--    player, centers the view on an arbitrary of these ships.
--
--    :arg player: The player to use
--    :type player: :class:`~wl.game.Player`
--    :arg items: An :class:`array` of :class:`tables` with `ware_or_worker_name = amount` pairs. As many ships will
--                be created as there are subtables, and the n-th ship created will load the
--                additional wares and workers defined in ``items[n]``. The capacity of each ship will
--                be adjusted to accommodate the build cost of the player's tribe's port building
--                plus one builder plus the number of additional items for this ship.
--
--    :returns: :const:`nil`

function launch_expeditions(player, items)
   local fields = wl.Game().map:find_ocean_fields(#items)

   local buildcost = 1
   local port = wl.Game():get_building_description(wl.Game():get_tribe_description(player.tribe_name).port)
   for s,i in pairs(port.buildcost) do buildcost = buildcost + i end

   for i,f in pairs(fields) do
      local ship = player:place_ship(f)

      local nr_i = 0
      for s,i in pairs(items[i]) do nr_i = nr_i + i end
      ship.capacity = buildcost + nr_i

      ship:make_expedition(items[i])
   end

   sleep_then_goto(player, 1000, fields[1])
end
