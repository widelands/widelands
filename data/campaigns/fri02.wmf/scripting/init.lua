-- =======================================================================
--                         Frisian Campaign Mission 2
-- =======================================================================
set_textdomain("scenario_fri02.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

p1 = wl.Game().players[1]
p2 = wl.Game().players[2] --Murilius – friend or foe
p3 = wl.Game().players[3] --Luw´Ci-Har – determined enemy
map = wl.Game().map

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"

--STORYLINE
-- Reebaud and his tribe have arrived in the North.
-- They only have a handful of supplies and soldiers. The first challenge is to
-- build a real economy with hardly any starting supplies.
-- There is virtually no fish. Aqua Farms are introduced.
-- There is nearly no metal and coal near the starting point. You will need 
-- charcoal burners for coal. A greater problem is iron: You have no basic 
-- swords to recruit soldiers. You must use the advanced weapons you have to 
-- train your few soldiers; recycling centres are introduced. The objective is 
-- to recruit a certain number of soldiers.
-- The empire tribe is met. Murilius has been sent to the cold Northlands to
-- found an outpost for the Empire. He demands your help to fight against a
-- vicious tribe of barbarians who are unwilling to share their lands with
-- anybody. You have a choice: Hand over masses of building materials, food and
-- advanced weapons/armour to gain a neutral ally (he won´t fight the barbarians
-- for you!), or refuse and have two powerful enemies?
-- The objective is to defeat the barbarians. Afterwards, Reebaud falls out with
-- Murilius (if they weren´t enemies already, and only if Murilius wasn´t 
-- defeated yet) over religion. The objective is to defeat Murilius.
-- Note: Both AI players have lots of resources, and they get generous starting
-- conditions. Murilius will not attack the barbarians if he can help it.
-- Teams: Blue   Frisians    1
--        Yellow Barbarians  -
--        Red    Empire      1 or - (depending on player´s choice)
--                           - later.
