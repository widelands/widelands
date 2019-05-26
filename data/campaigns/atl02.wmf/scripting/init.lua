-- =======================================================================
--                      Atlanteans Tutorial Mission 02
-- =======================================================================
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/table.lua"
include "scripting/ui.lua"


-- ===================
-- Constants & Config
-- ===================
set_textdomain("scenario_atl02.wmf")

game = wl.Game()
map = game.map
p1 = game.players[1]
kalitath = game.players[3]
maletus = game.players[2]

-- =================
-- global variables
-- =================

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

-- ====================
-- Starting the main thread
-- ====================

-- starting conditions are included later in the intro
include "map:scripting/mission_thread.lua"
