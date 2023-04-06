-- =======================================================================
--                         An endless game without rules
-- =======================================================================

include "scripting/win_conditions/win_condition_functions.lua"

push_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Endless Game"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Endless Game")
local wc_version = 1
local wc_desc = _("This is an endless game without rules.")
local r = {
   name = wc_name,
   description = wc_desc,
   peaceful_mode_allowed = true,
   func = function()
      local plrs = wl.Game().players

      -- set the objective with the game type for all players
      broadcast_objective("win_condition", wc_descname, wc_desc)

   end
}
pop_textdomain()
return r
