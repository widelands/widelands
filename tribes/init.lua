-- Tribes initialization
--
-- First the units are loaded, then the tribe descriptions
--
-- All tribes also have some basic information for the load screens
-- and the editor in preload.lua
--
-- Basic load order (first wares, then immovables etc.) is important,
-- because checks will be made in C++.
-- Also, enhanced/upgraded units need to come before their basic units.
--

set_textdomain("tribes")

tribes = wl.Tribes()


-- Adds 6 animations for each walking direction into 'table'. The pictures are
-- searched for in 'dirname'. All files should look like this
-- 'basename'_(e|ne|se|sw|w|nw)_\d+.png.
-- 'hotspot' is the hotspot for blitting.
-- 'animationname' is the name of the animation, e.g. "walkload"
-- 'fps' are the frames per second. Only use this if the animation has more than 1 frame.
function add_worker_animations(table, animationname, dirname, basename, hotspot, fps)
	if (fps ~= nil) then
		for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
			table[animationname .. "_" .. dir] = {
				template = basename .. "_" .. dir ..  "_??",
				directory = dirname,
				hotspot = hotspot,
				fps = fps,
			}
		end
	else
		for idx, dir in ipairs{ "ne", "e", "se", "sw", "w", "nw" } do
			table[animationname .. "_" .. dir] = {
				template = basename .. "_" .. dir ..  "_??",
				directory = dirname,
				hotspot = hotspot,
			}
		end
   end
end

-- NOCOM add_worker_animations(animations, "walkload", dirname, "walk", {13, 24}, 10)


-- ===================================
--    Ships
-- ===================================

print("Loading Ships")
include "tribes/ships/atlanteans/init.lua"
include "tribes/ships/barbarians/init.lua"
include "tribes/ships/empire/init.lua"


-- ===================================
--    Wares
-- ===================================

print("Loading Wares")
include "tribes/wares/armor/init.lua"
include "tribes/wares/armor_chain/init.lua"
include "tribes/wares/armor_gilded/init.lua"
include "tribes/wares/armor_helmet/init.lua"
include "tribes/wares/ax/init.lua"
include "tribes/wares/ax_battle/init.lua"
include "tribes/wares/ax_broad/init.lua"
include "tribes/wares/ax_bronze/init.lua"
include "tribes/wares/ax_sharp/init.lua"
include "tribes/wares/ax_warriors/init.lua"
include "tribes/wares/basket/init.lua"
include "tribes/wares/beer/init.lua"
include "tribes/wares/blackroot/init.lua"
include "tribes/wares/blackroot_flour/init.lua"
include "tribes/wares/blackwood/init.lua"
include "tribes/wares/bread_atlanteans/init.lua"
include "tribes/wares/bread_barbarians/init.lua"
include "tribes/wares/bread_empire/init.lua"
include "tribes/wares/bread_paddle/init.lua"
include "tribes/wares/buckets/init.lua"
include "tribes/wares/cloth/init.lua"
include "tribes/wares/coal/init.lua"
include "tribes/wares/corn/init.lua"
include "tribes/wares/cornmeal/init.lua"
include "tribes/wares/diamond/init.lua"
include "tribes/wares/felling_ax/init.lua"
include "tribes/wares/fire_tongs/init.lua"
include "tribes/wares/fish/init.lua"
include "tribes/wares/fishing_net/init.lua"
include "tribes/wares/fishing_rod/init.lua"
include "tribes/wares/flour/init.lua"
include "tribes/wares/gold/init.lua"
include "tribes/wares/gold_ore/init.lua"
include "tribes/wares/gold_thread/init.lua"
include "tribes/wares/granite/init.lua"
include "tribes/wares/grape/init.lua"
include "tribes/wares/grout/init.lua"
include "tribes/wares/hammer/init.lua"
include "tribes/wares/helmet/init.lua"
include "tribes/wares/helmet_mask/init.lua"
include "tribes/wares/helmet_warhelm/init.lua"
include "tribes/wares/hook_pole/init.lua"
include "tribes/wares/hunting_bow/init.lua"
include "tribes/wares/hunting_spear/init.lua"
include "tribes/wares/iron/init.lua"
include "tribes/wares/iron_ore/init.lua"
include "tribes/wares/kitchen_tools/init.lua"
include "tribes/wares/log/init.lua"
include "tribes/wares/marble/init.lua"
include "tribes/wares/marble_column/init.lua"
include "tribes/wares/meal/init.lua"
include "tribes/wares/meat/init.lua"
include "tribes/wares/milking_tongs/init.lua"
include "tribes/wares/pick/init.lua"
include "tribes/wares/planks/init.lua"
include "tribes/wares/quartz/init.lua"
include "tribes/wares/ration/init.lua"
include "tribes/wares/saw/init.lua"
include "tribes/wares/scythe/init.lua"
include "tribes/wares/shield_advanced/init.lua"
include "tribes/wares/shield_steel/init.lua"
include "tribes/wares/shovel/init.lua"
include "tribes/wares/smoked_fish/init.lua"
include "tribes/wares/smoked_meat/init.lua"
include "tribes/wares/snack/init.lua"
include "tribes/wares/spear/init.lua"
include "tribes/wares/spear_advanced/init.lua"
include "tribes/wares/spear_heavy/init.lua"
include "tribes/wares/spear_war/init.lua"
include "tribes/wares/spear_wooden/init.lua"
include "tribes/wares/spidercloth/init.lua"
include "tribes/wares/spider_silk/init.lua"
include "tribes/wares/stout/init.lua"
include "tribes/wares/tabard/init.lua"
include "tribes/wares/tabard_golden/init.lua"
include "tribes/wares/thatch_reed/init.lua"
include "tribes/wares/trident_double/init.lua"
include "tribes/wares/trident_heavy_double/init.lua"
include "tribes/wares/trident_light/init.lua"
include "tribes/wares/trident_long/init.lua"
include "tribes/wares/trident_steel/init.lua"
include "tribes/wares/water/init.lua"
include "tribes/wares/wheat/init.lua"
include "tribes/wares/wine/init.lua"
include "tribes/wares/wool/init.lua"


-- ===================================
--    Immovables
-- ===================================

print("Loading Immovables")
include "tribes/immovables/ashes/init.lua"
include "tribes/immovables/blackrootfield_harvested/init.lua"
include "tribes/immovables/blackrootfield_medium/init.lua"
include "tribes/immovables/blackrootfield_ripe/init.lua"
include "tribes/immovables/blackrootfield_small/init.lua"
include "tribes/immovables/blackrootfield_tiny/init.lua"
include "tribes/immovables/cornfield_harvested/init.lua"
include "tribes/immovables/cornfield_medium/init.lua"
include "tribes/immovables/cornfield_ripe/init.lua"
include "tribes/immovables/cornfield_small/init.lua"
include "tribes/immovables/cornfield_tiny/init.lua"
include "tribes/immovables/destroyed_building/init.lua"
include "tribes/immovables/field_harvested/init.lua"
include "tribes/immovables/field_medium/init.lua"
include "tribes/immovables/field_ripe/init.lua"
include "tribes/immovables/field_small/init.lua"
include "tribes/immovables/field_tiny/init.lua"
include "tribes/immovables/grapevine_medium/init.lua"
include "tribes/immovables/grapevine_ripe/init.lua"
include "tribes/immovables/grapevine_small/init.lua"
include "tribes/immovables/grapevine_tiny/init.lua"
include "tribes/immovables/reed_medium/init.lua"
include "tribes/immovables/reed_ripe/init.lua"
include "tribes/immovables/reed_small/init.lua"
include "tribes/immovables/reed_tiny/init.lua"
include "tribes/immovables/resi_coal1/init.lua"
include "tribes/immovables/resi_coal2/init.lua"
include "tribes/immovables/resi_gold1/init.lua"
include "tribes/immovables/resi_gold2/init.lua"
include "tribes/immovables/resi_iron1/init.lua"
include "tribes/immovables/resi_iron2/init.lua"
include "tribes/immovables/resi_none/init.lua"
include "tribes/immovables/resi_stones1/init.lua"
include "tribes/immovables/resi_stones2/init.lua"
include "tribes/immovables/resi_water1/init.lua"
include "tribes/immovables/shipconstruction_atlanteans/init.lua"
include "tribes/immovables/shipconstruction_barbarians/init.lua"
include "tribes/immovables/shipconstruction_empire/init.lua"


-- ===================================
--    Workers, Carriers & Soldiers
-- ===================================


print("Loading Workers")
include "tribes/workers/atlanteans/carrier/init.lua"
include "tribes/workers/atlanteans/armorsmith/init.lua"
include "tribes/workers/atlanteans/baker/init.lua"
include "tribes/workers/atlanteans/blackroot_farmer/init.lua"
include "tribes/workers/atlanteans/builder/init.lua"
include "tribes/workers/atlanteans/charcoal_burner/init.lua"
include "tribes/workers/atlanteans/farmer/init.lua"
include "tribes/workers/atlanteans/fishbreeder/init.lua"
include "tribes/workers/atlanteans/fisher/init.lua"
include "tribes/workers/atlanteans/forester/init.lua"
include "tribes/workers/atlanteans/geologist/init.lua"
include "tribes/workers/atlanteans/horse/init.lua"
include "tribes/workers/atlanteans/horsebreeder/init.lua"
include "tribes/workers/atlanteans/hunter/init.lua"
include "tribes/workers/atlanteans/miller/init.lua"
include "tribes/workers/atlanteans/miner/init.lua"
include "tribes/workers/atlanteans/sawyer/init.lua"
include "tribes/workers/atlanteans/scout/init.lua"
include "tribes/workers/atlanteans/shipwright/init.lua"
include "tribes/workers/atlanteans/smelter/init.lua"
include "tribes/workers/atlanteans/smoker/init.lua"
include "tribes/workers/atlanteans/soldier/init.lua"
include "tribes/workers/atlanteans/spiderbreeder/init.lua"
include "tribes/workers/atlanteans/stonecutter/init.lua"
include "tribes/workers/atlanteans/toolsmith/init.lua"
include "tribes/workers/atlanteans/trainer/init.lua"
include "tribes/workers/atlanteans/weaponsmith/init.lua"
include "tribes/workers/atlanteans/weaver/init.lua"
include "tribes/workers/atlanteans/woodcutter/init.lua"

include "tribes/workers/barbarians/carrier/init.lua"
include "tribes/workers/barbarians/baker/init.lua"
include "tribes/workers/barbarians/blacksmith_master/init.lua"
include "tribes/workers/barbarians/blacksmith/init.lua"
include "tribes/workers/barbarians/brewer_master/init.lua"
include "tribes/workers/barbarians/brewer/init.lua"
include "tribes/workers/barbarians/builder/init.lua"
include "tribes/workers/barbarians/cattlebreeder/init.lua"
include "tribes/workers/barbarians/charcoal_burner/init.lua"
include "tribes/workers/barbarians/farmer/init.lua"
include "tribes/workers/barbarians/fisher/init.lua"
include "tribes/workers/barbarians/gamekeeper/init.lua"
include "tribes/workers/barbarians/gardener/init.lua"
include "tribes/workers/barbarians/geologist/init.lua"
include "tribes/workers/barbarians/helmsmith/init.lua"
include "tribes/workers/barbarians/hunter/init.lua"
include "tribes/workers/barbarians/innkeeper/init.lua"
include "tribes/workers/barbarians/lime_burner/init.lua"
include "tribes/workers/barbarians/lumberjack/init.lua"
include "tribes/workers/barbarians/miner_master/init.lua"
include "tribes/workers/barbarians/miner_chief/init.lua"
include "tribes/workers/barbarians/miner/init.lua"
include "tribes/workers/barbarians/ox/init.lua"
include "tribes/workers/barbarians/ranger/init.lua"
include "tribes/workers/barbarians/scout/init.lua"
include "tribes/workers/barbarians/shipwright/init.lua"
include "tribes/workers/barbarians/smelter/init.lua"
include "tribes/workers/barbarians/soldier/init.lua"
include "tribes/workers/barbarians/stonemason/init.lua"
include "tribes/workers/barbarians/trainer/init.lua"
include "tribes/workers/barbarians/weaver/init.lua"

include "tribes/workers/empire/carrier/init.lua"
include "tribes/workers/empire/armorsmith/init.lua"
include "tribes/workers/empire/baker/init.lua"
include "tribes/workers/empire/brewer/init.lua"
include "tribes/workers/empire/builder/init.lua"
include "tribes/workers/empire/carpenter/init.lua"
include "tribes/workers/empire/charcoal_burner/init.lua"
include "tribes/workers/empire/donkey/init.lua"
include "tribes/workers/empire/donkeybreeder/init.lua"
include "tribes/workers/empire/farmer/init.lua"
include "tribes/workers/empire/fisher/init.lua"
include "tribes/workers/empire/forester/init.lua"
include "tribes/workers/empire/geologist/init.lua"
include "tribes/workers/empire/hunter/init.lua"
include "tribes/workers/empire/innkeeper/init.lua"
include "tribes/workers/empire/lumberjack/init.lua"
include "tribes/workers/empire/miller/init.lua"
include "tribes/workers/empire/miner_master/init.lua"
include "tribes/workers/empire/miner/init.lua"
include "tribes/workers/empire/pigbreeder/init.lua"
include "tribes/workers/empire/scout/init.lua"
include "tribes/workers/empire/shepherd/init.lua"
include "tribes/workers/empire/shipwright/init.lua"
include "tribes/workers/empire/smelter/init.lua"
include "tribes/workers/empire/soldier/init.lua"
include "tribes/workers/empire/stonemason/init.lua"
include "tribes/workers/empire/toolsmith/init.lua"
include "tribes/workers/empire/trainer/init.lua"
include "tribes/workers/empire/vinefarmer/init.lua"
include "tribes/workers/empire/weaponsmith/init.lua"
include "tribes/workers/empire/weaver/init.lua"


-- ===================================
--    Warehouses
-- ===================================

print("Loading Warehouses")
include "tribes/buildings/warehouses/atlanteans/headquarters/init.lua"
include "tribes/buildings/warehouses/atlanteans/port/init.lua"
include "tribes/buildings/warehouses/atlanteans/warehouse/init.lua"
include "tribes/buildings/warehouses/barbarians/headquarters/init.lua"
include "tribes/buildings/warehouses/barbarians/headquarters_interim/init.lua"
include "tribes/buildings/warehouses/barbarians/port/init.lua"
include "tribes/buildings/warehouses/barbarians/warehouse/init.lua"
include "tribes/buildings/warehouses/empire/headquarters/init.lua"
include "tribes/buildings/warehouses/empire/headquarters_shipwreck/init.lua"
include "tribes/buildings/warehouses/empire/port/init.lua"
include "tribes/buildings/warehouses/empire/warehouse/init.lua"


-- ===================================
--    Productionsites
-- ===================================

print("Loading Productionsites")
-- Atlanteans small
include "tribes/buildings/productionsites/atlanteans/quarry/init.lua"
include "tribes/buildings/productionsites/atlanteans/woodcutters_house/init.lua"
include "tribes/buildings/productionsites/atlanteans/foresters_house/init.lua"
include "tribes/buildings/productionsites/atlanteans/fishers_house/init.lua"
include "tribes/buildings/productionsites/atlanteans/fishbreeders_house/init.lua"
include "tribes/buildings/productionsites/atlanteans/hunters_house/init.lua"
include "tribes/buildings/productionsites/atlanteans/well/init.lua"
include "tribes/buildings/productionsites/atlanteans/gold_spinning_mill/init.lua"
include "tribes/buildings/productionsites/atlanteans/scouts_house/init.lua"
-- Atlanteans medium
include "tribes/buildings/productionsites/atlanteans/sawmill/init.lua"
include "tribes/buildings/productionsites/atlanteans/smokery/init.lua"
include "tribes/buildings/productionsites/atlanteans/mill/init.lua"
include "tribes/buildings/productionsites/atlanteans/bakery/init.lua"
include "tribes/buildings/productionsites/atlanteans/charcoal_kiln/init.lua"
include "tribes/buildings/productionsites/atlanteans/smelting_works/init.lua"
include "tribes/buildings/productionsites/atlanteans/shipyard/init.lua"
include "tribes/buildings/productionsites/atlanteans/toolsmithy/init.lua"
include "tribes/buildings/productionsites/atlanteans/weaponsmithy/init.lua"
include "tribes/buildings/productionsites/atlanteans/armorsmithy/init.lua"
-- Atlanteans big
include "tribes/buildings/productionsites/atlanteans/horsefarm/init.lua"
include "tribes/buildings/productionsites/atlanteans/farm/init.lua"
include "tribes/buildings/productionsites/atlanteans/blackroot_farm/init.lua"
include "tribes/buildings/productionsites/atlanteans/spiderfarm/init.lua"
include "tribes/buildings/productionsites/atlanteans/weaving_mill/init.lua"

-- Atlanteans mines
include "tribes/buildings/productionsites/atlanteans/crystalmine/init.lua"
include "tribes/buildings/productionsites/atlanteans/coalmine/init.lua"
include "tribes/buildings/productionsites/atlanteans/ironmine/init.lua"
include "tribes/buildings/productionsites/atlanteans/goldmine/init.lua"
-- Barbarians small
include "tribes/buildings/productionsites/barbarians/quarry/init.lua"
include "tribes/buildings/productionsites/barbarians/lumberjacks_hut/init.lua"
include "tribes/buildings/productionsites/barbarians/rangers_hut/init.lua"
include "tribes/buildings/productionsites/barbarians/fishers_hut/init.lua"
include "tribes/buildings/productionsites/barbarians/hunters_hut/init.lua"
include "tribes/buildings/productionsites/barbarians/gamekeepers_hut/init.lua"
include "tribes/buildings/productionsites/barbarians/well/init.lua"
include "tribes/buildings/productionsites/barbarians/scouts_hut/init.lua"
-- Barbarians medium
include "tribes/buildings/productionsites/barbarians/wood_hardener/init.lua"
include "tribes/buildings/productionsites/barbarians/lime_kiln/init.lua"
include "tribes/buildings/productionsites/barbarians/reed_yard/init.lua"
include "tribes/buildings/productionsites/barbarians/bakery/init.lua"
include "tribes/buildings/productionsites/barbarians/brewery/init.lua"
include "tribes/buildings/productionsites/barbarians/micro_brewery/init.lua"
include "tribes/buildings/productionsites/barbarians/big_inn/init.lua"
include "tribes/buildings/productionsites/barbarians/inn/init.lua"
include "tribes/buildings/productionsites/barbarians/tavern/init.lua"
include "tribes/buildings/productionsites/barbarians/charcoal_kiln/init.lua"
include "tribes/buildings/productionsites/barbarians/smelting_works/init.lua"
include "tribes/buildings/productionsites/barbarians/shipyard/init.lua"
include "tribes/buildings/productionsites/barbarians/warmill/init.lua"
include "tribes/buildings/productionsites/barbarians/axfactory/init.lua"
include "tribes/buildings/productionsites/barbarians/metal_workshop/init.lua"
-- Barbarians big
include "tribes/buildings/productionsites/barbarians/cattlefarm/init.lua"
include "tribes/buildings/productionsites/barbarians/farm/init.lua"
include "tribes/buildings/productionsites/barbarians/weaving_mill/init.lua"
include "tribes/buildings/productionsites/barbarians/helmsmithy/init.lua"
-- Barbarians mines
include "tribes/buildings/productionsites/barbarians/granitemine/init.lua"
include "tribes/buildings/productionsites/barbarians/coalmine_deeper/init.lua"
include "tribes/buildings/productionsites/barbarians/coalmine_deep/init.lua"
include "tribes/buildings/productionsites/barbarians/coalmine/init.lua"
include "tribes/buildings/productionsites/barbarians/ironmine_deeper/init.lua"
include "tribes/buildings/productionsites/barbarians/ironmine_deep/init.lua"
include "tribes/buildings/productionsites/barbarians/ironmine/init.lua"
include "tribes/buildings/productionsites/barbarians/goldmine_deeper/init.lua"
include "tribes/buildings/productionsites/barbarians/goldmine_deep/init.lua"
include "tribes/buildings/productionsites/barbarians/goldmine/init.lua"
-- Empire small
include "tribes/buildings/productionsites/empire/quarry/init.lua"
include "tribes/buildings/productionsites/empire/lumberjacks_house/init.lua"
include "tribes/buildings/productionsites/empire/foresters_house/init.lua"
include "tribes/buildings/productionsites/empire/fishers_house/init.lua"
include "tribes/buildings/productionsites/empire/hunters_house/init.lua"
include "tribes/buildings/productionsites/empire/well/init.lua"
include "tribes/buildings/productionsites/empire/scouts_house/init.lua"
-- Empire medium
include "tribes/buildings/productionsites/empire/stonemasons_house/init.lua"
include "tribes/buildings/productionsites/empire/sawmill/init.lua"
include "tribes/buildings/productionsites/empire/mill/init.lua"
include "tribes/buildings/productionsites/empire/bakery/init.lua"
include "tribes/buildings/productionsites/empire/brewery/init.lua"
include "tribes/buildings/productionsites/empire/vineyard/init.lua"
include "tribes/buildings/productionsites/empire/winery/init.lua"
include "tribes/buildings/productionsites/empire/inn/init.lua"
include "tribes/buildings/productionsites/empire/tavern/init.lua"
include "tribes/buildings/productionsites/empire/charcoal_kiln/init.lua"
include "tribes/buildings/productionsites/empire/smelting_works/init.lua"
include "tribes/buildings/productionsites/empire/shipyard/init.lua"
include "tribes/buildings/productionsites/empire/toolsmithy/init.lua"
include "tribes/buildings/productionsites/empire/armorsmithy/init.lua"
-- Empire big
include "tribes/buildings/productionsites/empire/donkeyfarm/init.lua"
include "tribes/buildings/productionsites/empire/farm/init.lua"
include "tribes/buildings/productionsites/empire/piggery/init.lua"
include "tribes/buildings/productionsites/empire/sheepfarm/init.lua"
include "tribes/buildings/productionsites/empire/weaving_mill/init.lua"
include "tribes/buildings/productionsites/empire/weaponsmithy/init.lua"
-- Empire mines
include "tribes/buildings/productionsites/empire/coalmine_deep/init.lua"
include "tribes/buildings/productionsites/empire/coalmine/init.lua"
include "tribes/buildings/productionsites/empire/ironmine_deep/init.lua"
include "tribes/buildings/productionsites/empire/ironmine/init.lua"
include "tribes/buildings/productionsites/empire/marblemine_deep/init.lua"
include "tribes/buildings/productionsites/empire/marblemine/init.lua"
include "tribes/buildings/productionsites/empire/goldmine_deep/init.lua"
include "tribes/buildings/productionsites/empire/goldmine/init.lua"


-- ===================================
--    Trainingsites
-- ===================================

print("Loading Trainingsites")
include "tribes/buildings/trainingsites/atlanteans/dungeon/init.lua"
include "tribes/buildings/trainingsites/atlanteans/labyrinth/init.lua"
include "tribes/buildings/trainingsites/barbarians/battlearena/init.lua"
include "tribes/buildings/trainingsites/barbarians/trainingcamp/init.lua"
include "tribes/buildings/trainingsites/empire/colosseum/init.lua"
include "tribes/buildings/trainingsites/empire/arena/init.lua"
include "tribes/buildings/trainingsites/empire/trainingcamp/init.lua"


-- ===================================
--    Militarysites
-- ===================================

print("Loading Militarysites")
include "tribes/buildings/militarysites/atlanteans/guardhouse/init.lua"
include "tribes/buildings/militarysites/atlanteans/guardhall/init.lua"
include "tribes/buildings/militarysites/atlanteans/tower_small/init.lua"
include "tribes/buildings/militarysites/atlanteans/tower_high/init.lua"
include "tribes/buildings/militarysites/atlanteans/tower/init.lua"
include "tribes/buildings/militarysites/atlanteans/castle/init.lua"

include "tribes/buildings/militarysites/barbarians/sentry/init.lua"
include "tribes/buildings/militarysites/barbarians/barrier/init.lua"
include "tribes/buildings/militarysites/barbarians/tower/init.lua"
include "tribes/buildings/militarysites/barbarians/citadel/init.lua"
include "tribes/buildings/militarysites/barbarians/fortress/init.lua"

include "tribes/buildings/militarysites/empire/sentry/init.lua"
include "tribes/buildings/militarysites/empire/blockhouse/init.lua"
include "tribes/buildings/militarysites/empire/barrier/init.lua"
include "tribes/buildings/militarysites/empire/outpost/init.lua"
include "tribes/buildings/militarysites/empire/tower/init.lua"
include "tribes/buildings/militarysites/empire/castle/init.lua"
include "tribes/buildings/militarysites/empire/fortress/init.lua"


-- ===================================
--    Partially Finished Buildings
-- ===================================

print("Loading Partially Finished Buildings")
include "tribes/buildings/partially_finished/constructionsite/init.lua"
include "tribes/buildings/partially_finished/dismantlesite/init.lua"


-- ===================================
--    Tribes
-- ===================================
print("Loading Atlanteans")
include "tribes/atlanteans.lua"
print("Loading Barbarians")
include "tribes/barbarians.lua"
print("Loading Empire")
include "tribes/empire.lua"

print("Finished loading tribes")
