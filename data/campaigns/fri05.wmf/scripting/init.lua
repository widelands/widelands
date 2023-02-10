-- =======================================================================
--                         Frisian Campaign Mission 5
-- =======================================================================
push_textdomain("scenario_fri05.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"
include "scripting/table.lua"
include "scripting/ui.lua"
include "scripting/messages.lua"

game = wl.Game()
map = game.map
p1 = game.players[1]
p2 = game.players[2]
difficulty = wl.Game().scenario_difficulty
wl.Game().allow_diplomacy = false

trade = {  -- {Wares given} vs {wares received}
   {
      {{barley = 1, fruit = 1}, {iron_ore = 1}},
      {{barley = 2, fruit = 2}, {gold_ore = 1}},
      {{fish = 2}, {granite = 1}},
      {{barley = 8, fruit = 8, fish = 8}, {granite = 4, iron_ore = 2, gold_ore = 2, log = 2, coal = 2}},
   },
   {
      {{barley = 1, fruit = 2}, {iron_ore = 1}},
      {{barley = 3, fruit = 2}, {gold_ore = 1}},
      {{fish = 2, honey = 1}, {granite = 1}},
      {{barley = 7, fruit = 7, fish = 7, honey = 7}, {granite = 3, iron_ore = 2, gold_ore = 1, log = 1, coal = 1}},
   },
   {
      {{barley = 2, fruit = 3}, {iron_ore = 1}},
      {{barley = 4, fruit = 5}, {gold_ore = 1}},
      {{fish = 3, honey = 2}, {granite = 1}},
   }
}
trade = trade[difficulty]

campaign_data = game:read_campaign_data("frisians", "fri04")
if not campaign_data then
   campaign_message_box({
      -- TRANSLATORS: Message title for a piece of information
      title = _("Note"),
      body = p(_([[You appear not to have completed the previous mission. You may still play this scenario, but you will be at a disadvantage. I recommend that you complete the previous scenario first and then restart this mission.]])),
      h = styles.get_size("campaign_message_box_h_min"),
   })
   campaign_data = {}
end

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
