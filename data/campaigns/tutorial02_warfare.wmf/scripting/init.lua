-- ================================================
--           Tutorial 02: Warfare
-- ================================================

plr = wl.Game().players[1]
map = wl.Game().map
mv = wl.ui.MapView()

push_textdomain("scenario_tutorial02_warfare.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/table.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"

wl.Game().allow_diplomacy = false
sf = map.player_slots[1].starting_field

citadel_field = map:get_field(32, 62)
f_hq = map:get_field(29, 55)
f_sentry_to_reinforce = map:get_field(37, 61)
f_sentry_to_dismantle = map:get_field(28, 57)

include "map:scripting/starting_conditions.lua"

include "map:scripting/texts.lua"
include "map:scripting/helper_functions.lua"
include "map:scripting/mission_thread.lua"
