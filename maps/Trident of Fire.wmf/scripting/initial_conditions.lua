

function init_human_player(player, f_hq, tribename)
	player:allow_buildings("all")
	if tribename == "barbarians" then
			prefilled_buildings(player, 
			{"headquarters_interim", f_hq.x, f_hq.y, 
				wares = {
					 log = 3,
				},
				workers = {
					 blacksmith = 1,
					 smelter = 1,
					 builder = 2,
					 carrier = 10,
					 gardener = 1,
					 geologist = 1,
					 ["lime-burner"] = 1,
					 lumberjack = 3,
					 miner = 2,
					 ranger = 2,
					 stonemason = 1,
					 hunter = 1,
					 farmer = 1,
					 fisher = 1,
					 baker = 1,
					 innkeeper = 1,
					 burner = 1
				},
				soldiers = {
					[{0,0,0,0}] = 3,		
				}
			}
		)
	end
	if tribename == "empire" then
		prefilled_buildings(player, { "headquarters_shipwreck", f_hq.x, f_hq.y,
		wares = {
			log = 2,
			wood = 3,
		},
		workers = {
			 baker = 1,
			 brewer = 2,
			 builder = 2,
			 burner = 1,
			 carpenter = 1,
			 carrier = 10,
			 farmer = 1,
			 fisher = 1,
			 forester = 2,
			 geologist = 1,
			 hunter = 1,
			 innkeeper = 1,
			 lumberjack = 2,
			 miller = 1,
			 miner = 3,
			 smelter = 1,
			 stonemason = 2,
			 toolsmith = 1,
			 vinefarmer =1
		  },
		  soldiers = {
			 [{0,0,0,0}] = 3,
		  }
	   })

	end		
	if tribename == "atlanteans" then
	    prefilled_buildings(player, { "headquarters", f_hq.x, f_hq.y,
		    wares = {
				log = 2,
				planks = 2,
				spidercloth = 1
			},
			workers = {
				 baker = 1,          
				 blackroot_farmer = 1,
				 builder = 2,
				 burner = 1,
				 carrier = 10,
				 farmer = 1,
				 fisher = 1,
				 forester = 2,
				 geologist = 1,
				 hunter = 1,
				 miller = 1,
				 miner = 6,
				 sawyer = 1,
				 smelter = 1,
				 smoker = 1,
				 spiderbreeder = 1,
				 stonecutter = 1,
				 toolsmith = 1,
				 woodcutter = 2,
				 weaver = 1
			},
			soldiers = {
				[{0,0,0,0}] = 3,
			}
	    })
	end
	return true
end

function init_AI_player(player, f_hq, f_port, f_vineyard, f_shipyard, tribename)
	if tribename == "barbarians" then
		player:allow_buildings("all")
		
		prefilled_buildings(player, 
			{ "port", f_hq.x, f_hq.y, 
			   wares = {
				  ax = 6,
				  bread_paddle = 2,
				  coal = 12,
				  fire_tongs = 2,
				  fish = 6,
				  grout = 12,
				  hammer = 12,
				  hunting_spear = 2,
				  iron = 12,
				  ironore = 5,
				  kitchen_tools = 4,
				  meal = 4,
				  meat = 6,
				  pick = 14,
				  pittabread = 8,
				  ration = 12,
				  raw_stone = 80,
				  scythe = 6,
				  shovel = 4,
				  snack = 3,
				  thatchreed = 24,
				  log = 150,
			   },
			   workers = {
				  carrier = 40,
				  blacksmith = 2,
				  brewer = 1,
				  builder = 10,
				  gardener = 1,
				  fisher = 2,
				  geologist = 4,
				  ["lime-burner"] = 1,
				  lumberjack = 3,
				  miner = 4,
				  ranger = 1,
				  stonemason = 2,
				  ox = 20, 
			   },
			   soldiers = { [{0,0,0,0}] = 45 },
			},
			{ "port", f_port.x, f_port.y},
			{ "shipyard", f_shipyard.x, f_shipyard.y, wares = {
				  blackwood = 10,
				  cloth = 4,
				  log = 2,
			   },
		 }
		)
	end	
		
	if tribename == "empire" then
		player:allow_buildings("all")

		prefilled_buildings(player, 
			{ "port", f_hq.x, f_hq.y, 
			   wares = {
				 helm = 4,
				 wood_lance = 5,
				 ax = 6,
				 bread_paddle = 2,
				 basket = 2,
				 bread = 8,
				 cloth = 5,
				 coal = 12,
				 fire_tongs = 2,
				 fish = 6,
				 fishing_rod = 2,
				 flour = 4,
				 gold = 4,
				 grape = 4,
				 hammer = 14,
				 hunting_spear = 2,
				 iron = 12,
				 ironore = 5,
				 kitchen_tools = 4,
				 marble = 60,
				 marblecolumn = 6,
				 meal = 4,
				 meat = 6,
				 pick = 14,
				 ration = 12,
				 saw = 2,
				 scythe = 5,
				 shovel = 6,
				 stone = 50,
				 log = 40,
				 water = 12,
				 wheat = 4,
				 wine = 8,
				 wood = 45,
				 wool = 2,
			   },
			   workers = {
				 armorsmith = 1,
				 brewer = 1,
				 builder = 10,
				 burner = 1,
				 carrier = 40,
				 geologist = 4,
				 lumberjack = 3,
				 miner = 4,
				 ["master-miner"] = 1,
				 stonemason = 2,
				 toolsmith = 2,
				 weaponsmith = 1,
				 donkey = 5,
			   },
			   soldiers = { [{0,0,0,0}] = 45 },
			},
			{ "port", f_port.x, f_port.y},
			{ "shipyard", f_shipyard.x, f_shipyard.y, 
			   wares = {
				 cloth = 4,
				 log = 2,
				 wood = 10,
			   }
		   },
		   { "vineyard", f_vineyard.x, f_vineyard.y}
		)
	end
	if tribename == "atlanteans" then
		player:allow_buildings("all")

		prefilled_buildings(player, { "port", f_hq.x, f_hq.y, 
		   wares = {
			 diamond = 7,
			 ironore = 5,
			 quartz = 9,
			 stone = 50,
			 spideryarn = 9,
			 log = 100,
			 coal = 12,
			 gold = 4,
			 goldyarn = 6,
			 iron = 8,
			 planks = 50,
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
			 bread_paddle = 2,
			 bucket = 2,
			 fire_tongs = 2,
			 fishing_net = 4,
			 hammer = 11,
			 hunting_bow = 1,
			 milking_tongs = 2,
			 hook_pole = 2,
			 pick = 12,
			 saw = 9,
			 scythe = 4,
			 shovel = 9,
			 tabard = 5,
			 light_trident = 5,
		   },
		   workers = {
			 armorsmith = 1,
			 blackroot_farmer = 1,
			 builder = 10,
			 burner = 1,
			 carrier = 40,
			 fish_breeder = 1,
			 geologist = 4,
			 miner = 4,
			 sawyer = 1,
			 stonecutter = 2,
			 toolsmith = 2,
			 weaponsmith = 1,
			 woodcutter = 3,
			 horse = 5,
		   },
		   soldiers = { [{0,0,0,0}] = 45 },
			},
			{ "port", f_port.x, f_port.y},
		   { "shipyard", f_shipyard.x, f_shipyard.y, wares = {
			  planks = 10,
			  spidercloth = 4,
			  log = 2,
		   }}
		)
	end	
	return true
end

