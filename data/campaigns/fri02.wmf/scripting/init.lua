-- =======================================================================
--                         Frisian Campaign Mission 2
-- =======================================================================
set_textdomain("scenario_fri02.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"
include "scripting/messages.lua"

game = wl.Game()
p1 = game.players[1] -- Reebaud – player’s tribe
p2 = game.players[2] -- Murilius – friend or foe
p3 = game.players[3] -- Kru-Gu’Lhar – determined enemy
map = game.map
p1_start = map.player_slots[1].starting_field

initial_soldiers = 20
campaign_data = game:read_campaign_data("frisians", "fri01")
if not campaign_data then
   campaign_message_box({
      title = _"Warning",
      body = p(_[[You appear not to have completed the previous mission. You may still play this scenario, but you will be at a disadvantage. I recommend that you complete the previous scenario first and then restart this mission.]]),
      w = 450,
      h = 150,
   })
   campaign_data = {}
   -- If he wants to cheat, let him, but give him a hard time
   initial_soldiers = 5
end

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
