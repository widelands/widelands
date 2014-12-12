# Comments are for graphic designers some ideas

[tribe]
# Workers positions in wares window. Columns are separated by ;,
# Entries in the columns separated by ,
workers_order=carrier, donkey, donkeybreeder;   stonemason, carpenter, lumberjack, forester, builder, shepherd, weaver, shipwright; fisher, hunter, farmer, miller, baker, brewer, pig-breeder, vinefarmer, innkeeper; geologist, miner, master-miner, charcoal_burner, smelter; toolsmith; soldier, trainer, weaponsmith, armorsmith, scout

# No idea for the frontier. Maybe some javelins?
[frontier]
pics=pics/frontier_??.png
hotspot=1 19

# Not just a plain color, maybe a cross or some stribes
[flag]
pics=pics/flag_??.png
hotspot=14 38
fps=10

[carrier types]
carrier=_Carrier
donkey=_Donkey

[soldier types]
soldier=_Soldier

[worker types]
armorsmith=_Armorsmith
baker=_Baker
brewer=_Brewer
builder=_Builder
charcoal_burner=_Charcoal Burner
carpenter=_Carpenter
farmer=_Farmer
fisher=_Fisher
forester=_Forester
geologist=_Geologist
hunter=_Hunter
innkeeper=_Innkeeper
lumberjack=_Lumberjack
master-miner=_Master Miner
miller=_Miller
miner=_Miner
pig-breeder=_Pig Breeder
shepherd=_Shepherd
shipwright=_Shipwright
smelter=_Smelter
stonemason=_Stonemason
toolsmith=_Toolsmith
trainer=_Trainer
weaponsmith=_Weaponsmith
weaver=_Weaver
vinefarmer=_Vine Farmer
scout=_Scout
donkeybreeder=_Donkey Breeder

[ship types]
ship=_Ship

[constructionsite types]
constructionsite=_Construction Site

[dismantlesite types]
dismantlesite=_Dismantle Site

[global militarysite types]
castle.atlanteans=_Castle
guardhall.atlanteans=_Guardhall
guardhouse.atlanteans=_Guardhouse
high_tower.atlanteans=_High Tower
small_tower.atlanteans=_Small Tower
tower.atlanteans=_Tower
barrier.barbarians=_Barrier
citadel.barbarians=_Citadel
donjon.barbarians=_Donjon
fortress.barbarians=_Fortress
sentry.barbarians=_Sentry

[warehouse types]
headquarters=_Headquarters
headquarters_shipwreck=_Headquarters Shipwreck
warehouse=_Warehouse
port=_Port

[productionsite types]
quarry=_Quarry
lumberjacks_house=_Lumberjack’s House
foresters_house=_Forester’s House
fishers_house=_Fisher’s House
hunters_house=_Hunter’s House
well=_Well
scouts_house=_Scout’s House
stonemasons_house=_Stonemason’s House
sawmill=_Sawmill
mill=_Mill
bakery=_Bakery
brewery=_Brewery
vineyard=_Vineyard
winery=_Winery
inn=_Inn
tavern=_Tavern
charcoal_kiln=_Charcoal Kiln
smelting_works=_Smelting Works
shipyard=_Shipyard
toolsmithy=_Toolsmithy
armorsmithy=_Armor Smithy
donkeyfarm=_Donkey Farm
sheepfarm=_Sheep Farm
weaving-mill=_Weaving Mill
piggery=_Piggery
farm=_Farm
weaponsmithy=_Weapon Smithy
deep_marblemine=_Deep Marble Mine
marblemine=_Marble Mine
deep_coalmine=_Deep Coal Mine
coalmine=_Coal Mine
deep_oremine=_Deep Iron Ore Mine
oremine=_Iron Ore Mine
deep_goldmine=_Deep Gold Mine
goldmine=_Gold Mine


[militarysite types]
sentry=_Sentry
barracks=_Barracks
barrier=_Barrier
outpost=_Outpost
tower=_Tower
castle=_Castle
fortress=_Fortress


[trainingsite types]
colosseum=_Colosseum
arena=_Arena
trainingcamp=_Training Camp


dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "empire",
   author = _"The Widelands Development Team",
   -- TRANSLATORS: This is a tribe name
   descname = _"_Empire",
   helptext = _"This is the culture of the Roman Empire.",
   bob_vision_range = 2,
   uiposition = 20,
   carrier2 = "donkey", -- NOCOM convert
   icon = pics/icon.png, -- NOCOM convert

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
		{
			-- Building Materials
			"granite",
			"log",
			"planks",
			"wool",
			"cloth"
		},
		{
			-- Food
			"fish",
			"meat",
			"water",
			"wheat",
			"flour",
			"bread_empire",
			"beer",
			"grape",
			"wine",
			"ration",
			"meal"
		},
		{
			-- Mining
			"marble",
			"marble_column",
			"coal",
			"iron_ore",
			"iron",
			"gold_ore",
			"gold"
		}
		{
			-- Tools
			"pick",
			"felling_ax",
			"saw",
			"shovel",
			"hammer",
			"fishing_rod",
			"hunting_spear",
			"scythe",
			"bread_paddle",
			"basket",
			"kitchen_tools",
			"fire_tongs"
		}
		{
			-- Weapons & Armor
			"spear_wooden",
			"spear",
			"spear_advanced",
			"spear_heavy",
			"spear_war",
			"armor_helmet",
			"armor",
			"armor_chain",
			"armor_gilded"
		}
	},
   immovables = {
		"ashes",
		"destroyed_building",
		"field_tiny",
		"field_small",
		"field_medium",
		"field_ripe",
		"field_harvested",
		"grapevine_tiny",
		"grapevine_small",
		"grapevine_medium",
		"grapevine_ripe",
		"resi_coal1",
		"resi_coal2",
		"resi_gold1",
		"resi_gold2",
		"resi_iron1",
		"resi_iron2",
		"resi_none",
		"resi_water",
		"resi_stones1",
		"resi_stones2",
		"shipconstruction_empire",
	}
}
