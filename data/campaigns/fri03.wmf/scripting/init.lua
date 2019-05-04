-- =======================================================================
--                         Frisian Campaign Mission 3
-- =======================================================================
set_textdomain("scenario_fri03.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

game = wl.Game()
map = game.map
p1 = game.players[1] -- Claus Lembeck     – Player's tribe
p2 = game.players[2] -- Waldemar Atterdag – Enemy, King of Denmark
p3 = game.players[3] -- Henneke Lembeck   – Claus's son
p4 = game.players[4] -- Hans Ravenstrupp  – Enemy to Claus's other son

ai_speed_1 = 500
ai_speed_2 = 10

p3_start = {
   map:get_field(346, 210),
   map:get_field(347, 210),
   map:get_field(344, 211),
   map:get_field(345, 211),
   map:get_field(346, 211),
   map:get_field(347, 211),
   map:get_field(343, 212),
   map:get_field(344, 212),
   map:get_field(345, 212),
   map:get_field(346, 212),
   map:get_field(343, 213),
   map:get_field(344, 213),
   map:get_field(345, 213),
   map:get_field(344, 214),
}

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
