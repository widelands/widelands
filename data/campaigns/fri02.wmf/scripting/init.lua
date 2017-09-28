-- =======================================================================
--                         Frisian Campaign Mission 2
-- =======================================================================
set_textdomain("scenario_fri02.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

p1 = wl.Game().players[1] --Reebaud – player´s tribe
p2 = wl.Game().players[2] --Murilius – friend or foe
p3 = wl.Game().players[3] --Kru-Gu´Lhar – determined enemy
map = wl.Game().map

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
