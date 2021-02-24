-- =======================================================================
--                      Scenario Trident of Fire
-- =======================================================================

include "scripting/coroutine.lua"
include "scripting/table.lua"
include "scripting/infrastructure.lua"
include "scripting/objective_utils.lua"
include "scripting/ui.lua"
include "scripting/set.lua"

game = wl.Game()
map = game.map

-- p1 = game.players[1]
-- p1.see_all = true

include "map:scripting/map_editing.lua"
include "map:scripting/initial_conditions.lua"

-- Identify the players which have been closed and must be replaced by AI players.
-- 'map.player_slots' always lists all 8 player slots, and its 'tribe_name' property tells us whether the slot is used or closed.
-- 'game.players' contains only the non-closed players. The indexing may therefore be off.
local nr_closed_slots = (#map.player_slots - #game.players)
hp1 = 0
hp2 = 0
hp3 = 0
AIp1 = 0
AIp2 = 0
AIp3 = 0
AIp4 = 4 - nr_closed_slots
AIp5 = 5 - nr_closed_slots
AIp6 = 6 - nr_closed_slots
local index = 0
if map.player_slots[1].tribe_name == "" then AIp1 = AIp4 else index = index + 1; hp1 = index end
if map.player_slots[2].tribe_name == "" then AIp2 = AIp5 else index = index + 1; hp2 = index end
if map.player_slots[3].tribe_name == "" then AIp3 = AIp6 else index = index + 1; hp3 = index end

-- field coordinates human players
hp1_f_hq = map:get_field(116,37)
hp2_f_hq = map:get_field(32, 9)
hp3_f_hq = map:get_field(139, 101)

-- field coordinates AI players
aip1_f_hq = map:get_field(116,37)
aip1_f_port = map:get_field(63,53)
aip1_f_vineyard = map:get_field(0,0)
aip1_f_shipyard = map:get_field(115, 30)
aip2_f_hq = map:get_field(44, 144)
aip2_f_port = map:get_field(82,139)
aip2_f_vineyard = map:get_field(47, 148)
aip2_f_shipyard = map:get_field(48,138)
aip3_f_hq = map:get_field(139, 101)
aip3_f_port = map:get_field(101,93)
aip3_f_vineyard = map:get_field(135,96)
aip3_f_shipyard = map:get_field(143, 95)
aip4_f_hq = map:get_field(28, 132)
aip4_f_port = map:get_field(73,118)
aip4_f_vineyard = map:get_field(25,135)
aip4_f_shipyard = map:get_field(33, 134)
aip5_f_hq = map:get_field(123, 11)
aip5_f_port = map:get_field(78,27)
aip5_f_vineyard = map:get_field(120,3)
aip5_f_shipyard = map:get_field(125, 15)
aip6_f_hq = map:get_field(11, 63)
aip6_f_port = map:get_field(57,68)
aip6_f_vineyard = map:get_field(11,54)
aip6_f_shipyard = map:get_field(7, 67)

-- if true then
if hp1 > 0 then
   a = init_human_player(game.players[hp1], hp1_f_hq, game.players[hp1].tribe_name)
end
if hp2 > 0 then
   a = init_human_player(game.players[hp2], hp2_f_hq, game.players[hp2].tribe_name)
end
if hp3 > 0 then
   a = init_human_player(game.players[hp3], hp3_f_hq, game.players[hp3].tribe_name)
end
if AIp1 > 0 then
   a = init_AI_player(game.players[AIp1], aip1_f_hq, aip1_f_port, aip1_f_vineyard, aip1_f_shipyard, game.players[AIp1].tribe_name)
end
if AIp2 > 0 then
   a = init_AI_player(game.players[AIp2], aip2_f_hq, aip2_f_port, aip2_f_vineyard, aip2_f_shipyard, game.players[AIp2].tribe_name)
end
if AIp3 > 0 then
   a = init_AI_player(game.players[AIp3], aip3_f_hq, aip3_f_port, aip3_f_vineyard, aip3_f_shipyard, game.players[AIp3].tribe_name)
end
if true then
   a = init_AI_player(game.players[AIp4], aip4_f_hq, aip4_f_port, aip4_f_vineyard, aip4_f_shipyard, game.players[AIp4].tribe_name)
   a = init_AI_player(game.players[AIp5], aip5_f_hq, aip5_f_port, aip5_f_vineyard, aip5_f_shipyard, game.players[AIp5].tribe_name)
   a = init_AI_player(game.players[AIp6], aip6_f_hq, aip6_f_port, aip6_f_vineyard, aip6_f_shipyard, game.players[AIp6].tribe_name)
end


run(flooding)
run(automatic_forester)
run(volcano_eruptions)
