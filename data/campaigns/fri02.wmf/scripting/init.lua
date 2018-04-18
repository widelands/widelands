-- =======================================================================
--                         Frisian Campaign Mission 2
-- =======================================================================
set_textdomain("scenario_fri02.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

game = wl.Game()
p1 = game.players[1] -- Reebaud – player’s tribe
p2 = game.players[2] -- Murilius – friend or foe
p3 = game.players[3] -- Kru-Gu’Lhar – determined enemy
map = game.map
p1_start = map.player_slots[1].starting_field

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
