-- =======================================================================
--                                 Player 1
-- =======================================================================
p1.see_all = true
p1:forbid_buildings("all")
p1:allow_buildings {
   "frisians_farm",
   "frisians_warehouse",
}
hq = p1:place_building("frisians_headquarters", map.player_slots [1].starting_field, false, true)
hq:set_wares {
   log = 40,
   granite = 50,
   thatch_reed = 50,
   brick = 80,
   clay = 30,
   water = 10,
   fish = 10,
   meat = 10,
   barley = 5,
   honey = 10,
   smoked_meat = 5,
   smoked_fish = 5,
   mead = 5,
   coal = 20,
   iron = 5,
   gold = 4,
   iron_ore = 10,
   bread_frisians = 15,
   honey_bread = 5,
   beer = 5,
   cloth = 5,
   fur = 10,
   fur_garment = 5,
   sword_short = 5,
   hammer = 5,
   fire_tongs = 2,
   bread_paddle = 2,
   kitchen_tools = 2,
   felling_ax = 3,
   needles = 2,
   basket = 2,
   pick = 5,
   shovel = 5,
   scythe = 3,
   hunting_spear = 2,
   fishing_net = 3,
}
hq:set_workers {
   frisians_blacksmith = 3,
   frisians_baker = 1,
   frisians_brewer = 1,
   frisians_builder = 5,
   frisians_charcoal_burner = 1,
   frisians_claydigger = 2,
   frisians_brickmaker = 2,
   frisians_carrier = 40,
   frisians_reed_farmer = 2,
   frisians_berry_farmer = 2,
   frisians_fruit_collector = 2,
   frisians_farmer = 1,
   frisians_landlady = 1,
   frisians_smoker = 1,
   frisians_geologist = 4,
   frisians_woodcutter = 3,
   frisians_beekeeper = 1,
   frisians_miner = 4,
   frisians_miner_master = 2,
   frisians_forester = 2,
   frisians_stonemason = 2,
   frisians_reindeer = 500,
   frisians_trainer = 3,
}
hq:set_soldiers {
   [{0,0,0,0}] = 40,
   [{1,3,1,0}] = 20,
   [{2,6,2,0}] = 10,
}

field = map.player_slots[1].starting_field.brn.brn.brn.brn.brn.brn.brn.brn.brn.brn.brn.brn.brn.brn.brn
p1:place_building("frisians_tower", field, false, true):set_soldiers({2, 6, 2, 0}, 7)
connected_road(p1, field.brn.immovable, "w,w,w|w,w,w|w,w,w|w,w,w|w,w,w")
for i=1,4 do
   field = field.ln.ln.ln
   p1:place_building("frisians_sentinel", field, false, true):set_soldiers({0, 0, 0, 0}, 2)
   connected_road(p1, field.brn.immovable, "ne,ne,ne|ne,ne,ne|ne,ne,ne|ne,ne,ne|ne,ne,ne")
end

field = field.ln.ln.ln
p1:place_building("frisians_tower", field, false, true):set_soldiers({2, 6, 2, 0}, 7)
for i=1,4 do
   field = field.tln.tln.tln
   p1:place_building("frisians_sentinel", field, false, true):set_soldiers({0, 0, 0, 0}, 2)
   connected_road(p1, field.brn.immovable, "e,e,e|e,e,e|e,e,e|e,e,e|e,e,e")
   connected_road(p1, field.brn.immovable, "ne,ne,ne|ne,ne,ne|ne,ne,ne|ne,ne,ne|ne,ne,ne")
end

field = field.tln.tln.tln
p1:place_building("frisians_tower", field, false, true):set_soldiers({2, 6, 2, 0}, 7)
connected_road(p1, field.brn.immovable, "ne,ne,ne|ne,ne,ne|ne,ne,ne|ne,ne,ne|ne,ne,ne")
for i=1,4 do
   field = field.trn.trn.trn
   p1:place_building("frisians_sentinel", field, false, true):set_soldiers({0, 0, 0, 0}, 2)
   connected_road(p1, field.brn.immovable, "e,e,e|e,e,e|e,e,e|e,e,e|e,e,e")
end

field = field.trn.trn.trn
p1:place_building("frisians_tower", field, false, true):set_soldiers({2, 6, 2, 0}, 7)
connected_road(p1, field.brn.immovable, "e,e,e|e,e,e|e,e,e|e,e,e|e,e,e")
for i=1,4 do
   field = field.rn.rn.rn
   p1:place_building("frisians_sentinel", field, false, true):set_soldiers({0, 0, 0, 0}, 2)
   connected_road(p1, field.brn.immovable, "sw,sw,sw|sw,sw,sw|sw,sw,sw|sw,sw,sw|sw,sw,sw")
end

field = field.rn.rn.rn
p1:place_building("frisians_tower", field, false, true):set_soldiers({2, 6, 2, 0}, 7)
for i=1,4 do
   field = field.brn.brn.brn
   p1:place_building("frisians_sentinel", field, false, true):set_soldiers({0, 0, 0, 0}, 2)
   connected_road(p1, field.brn.immovable, "w,w,w|w,w,w|w,w,w|w,w,w|w,w,w")
   connected_road(p1, field.brn.immovable, "sw,sw,sw|sw,sw,sw|sw,sw,sw|sw,sw,sw|sw,sw,sw")
end

field = field.brn.brn.brn
p1:place_building("frisians_tower", field, false, true):set_soldiers({2, 6, 2, 0}, 7)
connected_road(p1, field.brn.immovable, "sw,sw,sw|sw,sw,sw|sw,sw,sw|sw,sw,sw|sw,sw,sw")
for i=1,4 do
   field = field.bln.bln.bln
   p1:place_building("frisians_sentinel", field, false, true):set_soldiers({0, 0, 0, 0}, 2)
   connected_road(p1, field.brn.immovable, "w,w,w|w,w,w|w,w,w|w,w,w|w,w,w")
end

connected_road(p1, map.player_slots[1].starting_field.brn.immovable, "w,w,w|w,w,w|w,w,w|w,w,w|w,w,w")
connected_road(p1, map.player_slots[1].starting_field.brn.immovable, "e,e,e|e,e,e|e,e,e|e,e,e|e,e,e")
connected_road(p1, map.player_slots[1].starting_field.brn.immovable, "ne,ne,ne|ne,ne,ne|ne,ne,ne|ne,ne,ne|ne,ne,ne")
connected_road(p1, map.player_slots[1].starting_field.brn.immovable, "sw,sw,sw|sw,sw,sw|sw,sw,sw|sw,sw,sw|sw,sw,sw")

houses = {
   reed_farm = 2,
   well = 3,
   clay_pit = 2,
   brick_kiln = 2,
   furnace = 2,
   blacksmithy = 1,
   armor_smithy_small = 2,
   armor_smithy_large = 2,
   recycling_center = 1,
   sewing_room = 1,
   tailors_shop = 1,
   charcoal_kiln = 3,
   smokery = 3,
   bakery = 1,
   brewery = 1,
   honey_bread_bakery = 2,
   mead_brewery = 2,
   reindeer_farm = 2,
   barracks = 1,
   training_camp = 1,
   training_arena = 1,
}
fields = {}
field = map.player_slots[1].starting_field.ln.ln.ln
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.trn.trn.trn
table.insert(fields, field)
field = field.trn.trn.trn
table.insert(fields, field)
field = field.trn.trn.trn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.brn.brn.brn
table.insert(fields, field)
field = field.brn.brn.brn
table.insert(fields, field)
field = field.brn.brn.brn
table.insert(fields, field)
field = field.bln.bln.bln
table.insert(fields, field)
field = field.bln.bln.bln
table.insert(fields, field)
field = field.bln.bln.bln
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.tln.tln.tln
table.insert(fields, field)
field = field.tln.tln.tln
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.trn.trn.trn
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.trn.trn.trn
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.ln.ln.ln
table.insert(fields, field)
field = field.trn.trn.trn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = map.player_slots[1].starting_field.bln.bln.bln.bln.bln.bln
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
field = field.rn.rn.rn
table.insert(fields, field)
for bld,n in pairs(houses) do
   for i=1,n do
      field = math.random(#fields)
      local b = p1:place_building("frisians_" .. bld, fields[field], false, true)
      if b.valid_inputs then b:set_inputs(b.valid_inputs) end
      if b.valid_workers then b:set_workers(b.valid_workers) end
      table.remove(fields, field)
   end
end

for i=1,384 do
   field = nil
   while not field do
      field = map:get_field(math.random(5, map.width - 5), math.random(5, map.height - 5))
      if not p1:get_suitability("frisians_farm", field) then
         field = nil
      end
   end
   -- Using custom building here to prevent the player from destroying or
   -- dismantling some farms to make the connect_farms objective easier
   p1:place_building("frisians_farm_new", field, false, true):set_workers("frisians_farmer", 1)
end

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2.see_all = true
p2:allow_buildings("all")
p2:forbid_buildings {
   "empire_hunters_house",
   "empire_quarry",
   "empire_foresters_house",
   "empire_lumberjacks_house",
   "empire_port",
   "empire_shipyard",
   "empire_marblemine",
}

hq = p2:place_building("HQ_atterdag", map.player_slots [2].starting_field, false, true)
hq:set_wares {
   armor_helmet = 4,
   spear_wooden = 5,
   felling_ax = 6,
   bread_paddle = 2,
   basket = 2,
   empire_bread = 8,
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
   iron_ore = 5,
   kitchen_tools = 4,
   marble = 25,
   marble_column = 6,
   meat = 6,
   pick = 8,
   saw = 2,
   scythe = 5,
   shovel = 6,
   granite = 40,
   log = 30,
   water = 12,
   wheat = 4,
   wine = 8,
   planks = 45,
   wool = 2,
}
hq:set_workers {
   empire_armorsmith = 1,
   empire_brewer = 1,
   empire_builder = 10,
   empire_carrier = 40,
   empire_charcoal_burner = 1,
   empire_geologist = 4,
   empire_lumberjack = 3,
   empire_miner = 4,
   empire_stonemason = 2,
   empire_toolsmith = 2,
   empire_trainer = 3,
   empire_weaponsmith = 1,
   empire_donkey = 5,
}
hq:set_soldiers {
   [{0,0,0,0}] = 160,
   [{0,0,0,2}] = 80,
   [{2,2,0,0}] = 80,
   [{4,4,0,2}] = 40,
}



-- =======================================================================
--                                 Player 3
-- =======================================================================

-- He'll join the game later
p3.see_all = true
p3:forbid_buildings("all")

-- =======================================================================
--                                 Player 4
-- =======================================================================
p4.see_all = true
p4:allow_buildings("all")
p4:forbid_buildings {
   "barbarians_hunters_hut",
   "barbarians_gamekeepers_hut",
   "barbarians_quarry",
   "barbarians_rangers_hut",
   "barbarians_lumberjacks_hut",
   "barbarians_inn",
   "barbarians_port",
   "barbarians_shipyard",
}

hq = p4:place_building("barbarians_headquarters", map.player_slots [4].starting_field, false, true)
hq:set_wares {
   ax = 5,
   bread_paddle = 2,
   blackwood = 32,
   cloth = 5,
   coal = 12,
   felling_ax = 4,
   fire_tongs = 2,
   fish = 6,
   fishing_rod = 2,
   gold = 4,
   grout = 12,
   hammer = 12,
   hunting_spear = 2,
   iron = 12,
   iron_ore = 5,
   kitchen_tools = 4,
   meat = 6,
   pick = 8,
   barbarians_bread = 8,
   granite = 40,
   scythe = 6,
   shovel = 4,
   thatch_reed = 24,
   log = 80,
}
hq:set_workers {
   barbarians_blacksmith = 2,
   barbarians_brewer = 1,
   barbarians_builder = 10,
   barbarians_charcoal_burner = 1,
   barbarians_carrier = 40,
   barbarians_gardener = 1,
   barbarians_geologist = 4,
   barbarians_lime_burner = 1,
   barbarians_lumberjack = 3,
   barbarians_miner = 4,
   barbarians_miner_master = 4,
   barbarians_ranger = 1,
   barbarians_stonemason = 2,
   barbarians_trainer = 3,
   barbarians_ox = 5,
}
hq:set_soldiers {
   [{0,0,0,0}] = 160,
   [{0,0,0,2}] = 80,
   [{3,5,0,0}] = 80,
   [{3,5,0,2}] = 40,
}


