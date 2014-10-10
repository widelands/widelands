-- =======================================================================
--                       Barbarians Campaign Mission 3
-- =======================================================================

set_textdomain("scenario_t03.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

game = wl.Game()
p1 = game.players[1]
p2 = game.players[2]
p3 = game.players[3]
p4 = game.players[4]

include "map:scripting/starting_conditions.lua"
include "map:scripting/texts.lua"

-- Some messages that spare some typing
function send_msg(msg)
   msg.h = 400
   p1:message_box(msg.title, msg.body, msg)
end
function add_obj(t)
   return p1:add_objective(t.name, t.title, t.body)
end

include "map:scripting/mission_thread.lua"
