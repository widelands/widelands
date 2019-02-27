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

include "map:scripting/texts.lua"

total_soldiers = 25
takeover_soldiers = 10
campaign_data = game:read_campaign_data("frisians", "fri01")
if not campaign_data then
   campaign_message_box(campaign_data_warning)
   campaign_data = {}
   -- If he wants to cheat, let him, but give him a hard time
   total_soldiers = 5
   takeover_soldiers = 0
end

include "map:scripting/mission_thread.lua"
