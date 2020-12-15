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

p1.allow_additional_expedition_items = false

port_south = map:get_field(6, 461)
port_volcano = map:get_field(16, 402)
port_desert_s = map:get_field(42, 243)
port_desert_n = map:get_field(4, 235)
port_north = map:get_field(20, 162)

local field = map:get_field(1, 456)
obstacles_1 = {field}
for i=1, 14 do
   field = field.brn
   table.insert(obstacles_1, field)
end
-- NOCOM: The obstacle-related scripting <2> and <3> is to ensure that the red and yellow
-- players don't reach the port spaces too soon. If testers report that this is a problem,
-- this code will be uncommented, else removed.
-- (Note to self: Don't forget to adjust mission_thread and starting_conditions!)
-- obstacles_1 is used to ensure the player doesn’t reach the first port space
-- before he owns the whole southern island.

-- obstacles_2 = {
--    map:get_field(19, 401),
--    map:get_field(21, 399),
--    map:get_field(22, 397),
--    map:get_field(23, 397),
--    map:get_field(25, 395),
--    map:get_field(26, 393),
--    map:get_field(27, 389),
--    map:get_field(28, 389),
--    map:get_field(30, 389),
--    map:get_field(31, 386),
--    map:get_field(29, 384),
--    map:get_field(31, 383),
-- }
-- field = map:get_field(15, 164)
-- obstacles_3 = {field}
-- for i=1, 11 do
--    field = field.trn
--    table.insert(obstacles_3, field)
-- end
-- for i=1, 10 do
--    field = field.brn
--    table.insert(obstacles_3, field)
-- end

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
