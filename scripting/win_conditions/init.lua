-- This config file sets the order of the starting conditions
dirname = path.dirname(__file__)

return {
	dirname .. "02_collectors.lua",
	dirname .. "04_wood_gnome.lua",
	dirname .. "03_territorial_time.lua",
	dirname .. "03_territorial_lord.lua",
	dirname .. "01_defeat_all.lua",
	dirname .. "00_endless_game.lua",
	dirname .. "05_endless_game_fogless.lua",
}
