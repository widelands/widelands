-- This config file sets the order of the starting conditions
local dirname = path.dirname(__file__)

return {
   dirname .. "collectors.lua",
   dirname .. "wood_gnome.lua",
   dirname .. "artifacts.lua",
   dirname .. "territorial_time.lua",
   dirname .. "territorial_lord.lua",
   dirname .. "hq_hunter.lua",
   dirname .. "defeat_all.lua",
   dirname .. "endless_game.lua",
}
