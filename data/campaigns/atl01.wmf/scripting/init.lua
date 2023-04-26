-- =======================================================================
--                      Atlanteans Tutorial Mission 01
-- =======================================================================

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/set.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

include "map:scripting/water_rising.lua"

-- ===================
-- Constants & Config
-- ===================
push_textdomain("scenario_atl01.wmf")

game = wl.Game()
map = game.map
p1 = game.players[1]
first_tower_field = map:get_field(94, 149)
second_tower_field = map:get_field(79, 150)

game.allow_diplomacy = false

-- =================
-- global variables
-- =================
expand_objective = nil
let_the_water_rise = false -- If set to true, the water will begin to rise

include "map:scripting/texts.lua"

-- =================
-- Helper functions
-- =================

-- Show many message boxes
function msg_boxes(boxes_descr)
   for idx,box_descr in ipairs(boxes_descr) do
      campaign_message_box(box_descr)
   end
end

-- Return the total number of items in warehouses of the given
-- ware.
function count_in_warehouses(ware)
   local whs = array_combine(
      p1:get_buildings("atlanteans_headquarters"),
      p1:get_buildings("atlanteans_warehouse")
   )
   local rv = 0
   for idx,wh in ipairs(whs) do
      rv = rv + wh:get_wares(ware)
   end
   return rv
end

-- starting conditions are included later in the intro
include "map:scripting/mission_thread.lua"
include "map:scripting/water_rising.lua"
