-- =======================================================================
--                         Frisian Campaign Mission 3
-- =======================================================================
push_textdomain("scenario_fri03.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"
include "scripting/messages.lua"

game = wl.Game()
map = game.map
p1 = game.players[1] -- Claus Lembeck           – Player's tribe
p2 = game.players[2] -- Angadthur               – Atlantean King
p3 = game.players[3] -- Marcus Caius Iniucundus – Imperial Legate
difficulty = wl.Game().scenario_difficulty
wl.Game().allow_diplomacy = false

port_south = map:get_field(6, 461)
port_volcano = map:get_field(16, 402)
port_desert_s = map:get_field(42, 243)
port_desert_n = map:get_field(4, 235)
port_north = map:get_field(20, 162)

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
