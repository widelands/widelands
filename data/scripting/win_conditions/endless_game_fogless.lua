-- =======================================================================
--                         An endless game without rules
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/win_conditions/win_condition_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Endless Game (no fog)"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Endless Game (no fog)")
local wc_desc = _ "This is an endless game without rules. Fog of war is disabled."
local wc_version = 1
return {
   name = wc_name,
   description = wc_desc,
   peaceful_mode_allowed = true,
   func = function()
      local plrs = wl.Game().players

      -- set the objective with the game type for all players
      broadcast_objective("win_condition", wc_descname, wc_desc)

      -- reveal the whole map for every player
      local game = wl.Game()
      local map = game.map
      local fields = {}
      local i = 1
      for x=0, map.width-1 do
         for y=0, map.height-1 do
            fields[i] = map:get_field(x,y)
            i = i + 1
         end
      end
      for idx, plr in ipairs(game.players) do
         plr:reveal_fields(fields)
      end

      -- Iterate all players, if one is defeated, remove him
      -- from the list, send him a defeated message and give him full vision
      repeat
         sleep(5000)
         check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
      until count_factions(plrs) < 1

   end
}
