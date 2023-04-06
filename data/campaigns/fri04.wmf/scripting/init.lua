-- =======================================================================
--                         Frisian Campaign Mission 4
-- =======================================================================
push_textdomain("scenario_fri04.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

game = wl.Game()
p1 = game.players[1] -- Claus Lembeck – player’s tribe
p2 = game.players[2] -- Reebaud – friend and ally
p3 = game.players[3] -- Marcus Caius Iniucundus – enemy (if present)
p4 = game.players[4] -- Linthesilea – intruderess
map = game.map
difficulty = wl.Game().scenario_difficulty
wl.Game().allow_diplomacy = false

campaign_data = game:read_campaign_data("frisians", "fri03")
if not campaign_data then
   campaign_message_box({
      -- TRANSLATORS: Message title for a piece of information
      title = _("Note"),
      body = p(_([[You appear not to have completed the previous mission. You may still play this scenario, but you will be at a disadvantage. I recommend that you complete the previous scenario first and then restart this mission.]])),
      h = styles.get_size("campaign_message_box_h_min"),
   })
   -- If he wants to cheat, let him, but give him a really hard time
   campaign_data = {soldiers = {}, payment = 1000}
end

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
