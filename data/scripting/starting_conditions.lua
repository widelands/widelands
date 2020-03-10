include "scripting/coroutine.lua"
include "scripting/ui.lua"

function sleep_then_goto(sleeptime, field)
   run(function()
      sleep(sleeptime)
      scroll_to_field(field)
   end)
end

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

   if player.number == wl.Game().interactive_player then sleep_then_goto(1000, fields[1]) end
end
