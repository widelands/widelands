game = wl.Game()
map = game.map

p1 = game.players[1]
p2 = game.players[2]


use("aux", "infrastructure")
use("aux", "coroutine")
use("aux", "win_condition_functions")

function initialize()
   sleep(10)

   -------------------
   -- player set up --
   -------------------

   port11 = p1:place_building("port", map:get_field(139, 105), false, true)
   port12 = p1:place_building("port", map:get_field(37, 75), false, true)
   prefilled_buildings(p1,
      { "tower", 140, 101 },
      { "tower", 36, 72 },
      { "shipyard", 137, 103 }
   )

   port21 = p2:place_building("port", map:get_field(38, 103), false, true)
   port22 = p2:place_building("port", map:get_field(150, 79), false, true)
   prefilled_buildings(p2,
      { "tower", 33, 101 },
      { "tower", 149, 76 },
      { "shipyard", 33, 106 }
   )

port11:set_wares{
         stone = 30,
         spideryarn = 9,
         trunk = 20,
         coal = 5,
         goldyarn = 2,
         iron = 8,
         planks = 35,
         spidercloth = 5,
         blackroot = 5,
         blackrootflour = 12,
         bread = 8,
         corn = 5,
         cornflour = 12,
         fish = 3,
         meat = 3,
         smoked_fish = 6,
         smoked_meat = 6,
         water = 12,
         bakingtray = 2,
         bucket = 2,
         fishing_net = 4,
         hammer = 2,
         hunting_bow = 1,
         milking_tongs = 2,
         hook_pole = 2,
         pick = 6,
         saw = 9,
         scythe = 4,
         shovel = 9,
         tabard = 3,
         light_trident = 3,
}
port12:set_wares{
         diamond = 7,
         ironore = 5,
         quartz = 9,
         stone = 20,
         spideryarn = 9,
         trunk = 20,
         coal = 12,
         gold = 4,
         goldyarn = 4,
         iron = 8,
         planks = 25,
         spidercloth = 5,
         bread = 8,
         smoked_fish = 6,
         smoked_meat = 6,
         fire_tongs = 2,
         hammer = 11,
         pick = 8,
         saw = 2,
         shovel = 2,
         tabard = 2,
         light_trident = 2,
}
port11:set_workers{
         blackroot_farmer = 1,
         builder = 5,
         burner = 1,
         carrier = 40,
         fish_breeder = 1,
         sawyer = 1,
         stonecutter = 2,
         toolsmith = 1,
         weaponsmith = 1,
         woodcutter = 3,
         horse = 5,
}
port12:set_workers{
         armoursmith = 1,
         builder = 5,
         carrier = 40,
         geologist = 4,
         miner = 4,
         sawyer = 1,
         stonecutter = 2,
         toolsmith = 1,
         woodcutter = 1,
         horse = 2,
}
port11:set_soldiers({0,0,0,0}, 15)
port12:set_soldiers({0,0,0,0}, 10)

port21:set_wares{
         stone = 30,
         spideryarn = 9,
         trunk = 20,
         coal = 5,
         goldyarn = 2,
         iron = 8,
         planks = 35,
         spidercloth = 5,
         blackroot = 5,
         blackrootflour = 12,
         bread = 8,
         corn = 5,
         cornflour = 12,
         fish = 3,
         meat = 3,
         smoked_fish = 6,
         smoked_meat = 6,
         water = 12,
         bakingtray = 2,
         bucket = 2,
         fishing_net = 4,
         hammer = 2,
         hunting_bow = 1,
         milking_tongs = 2,
         hook_pole = 2,
         pick = 6,
         saw = 9,
         scythe = 4,
         shovel = 9,
         tabard = 3,
         light_trident = 3,
}
port22:set_wares{
         diamond = 7,
         ironore = 5,
         quartz = 9,
         stone = 20,
         spideryarn = 9,
         trunk = 20,
         coal = 12,
         gold = 4,
         goldyarn = 4,
         iron = 8,
         planks = 25,
         spidercloth = 5,
         bread = 8,
         smoked_fish = 6,
         smoked_meat = 6,
         fire_tongs = 2,
         hammer = 11,
         pick = 8,
         saw = 2,
         shovel = 2,
         tabard = 2,
         light_trident = 2,
}
port21:set_workers{
         blackroot_farmer = 1,
         builder = 5,
         burner = 1,
         carrier = 40,
         fish_breeder = 1,
         sawyer = 1,
         stonecutter = 2,
         toolsmith = 1,
         weaponsmith = 1,
         woodcutter = 3,
         horse = 5,
}
port22:set_workers{
         armoursmith = 1,
         builder = 5,
         carrier = 40,
         geologist = 4,
         miner = 4,
         sawyer = 1,
         stonecutter = 2,
         toolsmith = 1,
         woodcutter = 1,
         horse = 2,
}
port21:set_soldiers({0,0,0,0}, 15)
port22:set_soldiers({0,0,0,0}, 10)

   -------------------
   -- Win condition --
   -------------------

	local plrs = wl.Game().players
	set_textdomain("mp_scenario_together_we_re_strong.pot")

	broadcast(plrs, _ "Together we're strong", _ "The player who defeats the other player wins the game")
	sleep(240000)

	-- Iterate all players, if one is defeated, remove him
	-- from the list, send him a defeated message and give him full vision
	repeat
		sleep(5000)
		check_player_defeated(plrs, _ "You are defeated!",
			_ ("You have nothing to command left. If you want, you may continue as spectator."))
	until count_factions(plrs) <= 1

	-- Send congratulations to all remaining players
	broadcast(plrs, _ "Congratulations!", _ "You have won this game!", {popup = true})
end

run(initialize)

