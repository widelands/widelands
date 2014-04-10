-- =======================================================================
--                         Empire Campaign Mission 1
-- =======================================================================
set_textdomain("scenario_emp01.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

p1 = wl.Game().players[1]

include "map:scripting/starting_conditions.lua"
include "map:scripting/texts.lua"

-- Some messages that spare some typing
function send_msg(msg)
   if not msg.h then msg.h = 400 end
   p1:message_box(msg.title, msg.body, msg)
end
function add_obj(t)
   return p1:add_objective(t.name, t.title, t.body)
end

include "map:scripting/mission_thread.lua"
