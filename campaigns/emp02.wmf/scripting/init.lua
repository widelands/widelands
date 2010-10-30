-- =======================================================================
--                         Empire Campaign Mission 2                        
-- =======================================================================
set_textdomain("scenario_emp02.wmf")

use("aux", "coroutine")
use("aux", "objective_utils")
use("aux", "infrastructure")
use("aux", "table")
use("aux", "ui")

game = wl.Game()
p1 = game.players[1]
p2 = game.players[2]

use("map", "starting_conditions")
use("map", "texts")

-- Some messages that spare some typing
function send_msg(msg)
   p1:message_box(msg.title, msg.body, msg)
end
function add_obj(t)
   return p1:add_objective(t.name, t.title, t.body)
end

use("map", "mission_thread")
