-- =======================================================================
--                      Atlanteans Tutorial Mission 01
-- =======================================================================

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

include "map:scripting/water_rising.lua"

-- ===================
-- Constants & Config
-- ===================
set_textdomain("scenario_atl01.wmf")

game = wl.Game()
map = game.map
p1 = game.players[1]
first_tower_field = map:get_field(94, 149)
second_tower_field = map:get_field(79, 150)

-- =================
-- global variables
-- =================
expand_objective = nil
let_the_water_rise = false -- If set to true, the water will begin to rise

include "map:scripting/texts.lua"

-- =================
-- Helper functions
-- =================
-- Show one message box
function msg_box(i)
   if i.pre_func then i.pre_func() end

   if not i.h then i.h = 400 end

   message_box(p1, i.title, i.body, i)

   if i.post_func then i.post_func() end

   sleep(130)
end

-- Show many message boxes
function msg_boxes(boxes_descr)
   for idx,box_descr in ipairs(boxes_descr) do
      msg_box(box_descr)
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

function send_building_lost_message(f)
   send_message(
      p1,
      -- TRANSLATORS: Short message title. Translate as "Lost!" if you don't have enough space.
      pgettext("message_short_title", "Building lost!"),
      rt("image=".. f.immovable.descr.representative_image,
         p(_"We lost a building to the ocean!")
      ),
      {
         field = f,
         popup = false,
         icon = f.immovable.descr.representative_image,
         heading = pgettext("message_heading", "Building lost!")
      }
   )
end

-- starting conditions are included later in the intro
include "map:scripting/mission_thread.lua"
include "map:scripting/water_rising.lua"
