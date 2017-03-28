-- =======================================================================
--                         Empire Campaign Mission 3
-- =======================================================================
set_textdomain("scenario_emp03.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"


p1 = wl.Game().players[1]
p2 = wl.Game().players[2]

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"

map = wl.Game().map
mv = wl.ui.MapView()
sf = map.player_slots[1].starting_field

function count_in_warehouses(ware)
   local whs = array_combine(
      p1:get_buildings("empire_headquarters_shipwreck"),
      p1:get_buildings("empire_warehouse"),
      p1:get_buildings("empire_port")
   )
   local rv = 0
   for idx,wh in ipairs(whs) do
      rv = rv + wh:get_wares(ware)
   end
   return rv
end
