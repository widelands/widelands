-- =======================================================================
--                         Frisian Campaign Mission 2
-- =======================================================================
push_textdomain("scenario_fri02.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

game = wl.Game()
p1 = game.players[1] -- Reebaud – player’s tribe
p2 = game.players[2] -- Murilius – friend or foe
p3 = game.players[3] -- Kru-Gu’Lhar – determined enemy
map = game.map
p1_start = map.player_slots[1].starting_field
difficulty = wl.Game().scenario_difficulty
wl.Game().allow_diplomacy = false

total_soldiers = {40, 25, 10}
total_soldiers = total_soldiers[difficulty]
takeover_soldiers = {15, 10, 5}
takeover_soldiers = takeover_soldiers[difficulty]

campaign_data = game:read_campaign_data("frisians", "fri01")
if not campaign_data then
   campaign_message_box({
      -- TRANSLATORS: Message title for a piece of information
      title = _("Note"),
      body = p(_([[You appear not to have completed the previous mission. You may still play this scenario, but you will be at a disadvantage. I recommend that you complete the previous scenario first and then restart this mission.]])),
      h = styles.get_size("campaign_message_box_h_min"),
   })
   campaign_data = { wares = {}, workers = {}, port_soldiers = {}, ship_soldiers = {} }
   -- If he wants to cheat, let him, but give him a hard time
   total_soldiers = 5
   takeover_soldiers = 0
elseif not campaign_data.port_soldiers then
   -- Convert old-style campaign data format
   campaign_data = {
      port_soldiers = campaign_data,
      ship_soldiers = {}, wares = {}, workers = {}
   }
end

has_gold = 0
for name,amount in pairs(campaign_data.wares) do
   if name == "pick" then
      lost_miners = true
   elseif name == "iron" or name == "iron_ore" or name == "scrap_metal_mixed" or name == "scrap_iron" then
      lost_metals = true
   elseif name == "gold" or name == "gold_ore" then
      has_gold = has_gold + amount
   end
end
for name,amount in pairs(campaign_data.workers) do
   if name == "frisians_miner" or name == "frisians_miner_master" then
      lost_miners = true
      break
   end
end

include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
