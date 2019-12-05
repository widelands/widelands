-- RST
-- init.lua
-- -----------------
--
-- Tribes initialization
--
-- First the units are loaded, then the tribe descriptions.
--
-- All tribes also have some basic information for the load screens
-- and the editor in ``preload.lua``.
--
-- Basic load order (first wares, then immovables etc.) is important,
-- because checks will be made in C++.
-- Also, enhanced/upgraded units need to come before their basic units.
--

if wl.Game then egbase = wl.Game() else egbase = wl.Editor() end

tribes = wl.Tribes()
include "scripting/mapobjects.lua"

print("┏━ Running Lua for tribes:")

print_loading_message("┗━ took", function()
   -- ===================================
   --    Ships
   -- ===================================

   print_loading_message("┃    Ships", function()
      egbase:set_loading_message(_("Loading tribes: Ships (1/15)"))
      include "tribes/ships/atlanteans/init.lua"
      include "tribes/ships/barbarians/init.lua"
      include "tribes/ships/empire/init.lua"
      include "tribes/ships/frisians/init.lua"
   end)

   -- ===================================
   --    Wares
   -- ===================================

   print_loading_message("┃    Wares", function()
      egbase:set_loading_message(_("Loading tribes: Wares (2/15)"))

      include "tribes/wares/clay/init.lua"
      include "tribes/wares/brick/init.lua"
      include "tribes/wares/fruit/init.lua"
      include "tribes/wares/honey/init.lua"
      include "tribes/wares/barley/init.lua"
      include "tribes/wares/bread_frisians/init.lua"
      include "tribes/wares/honey_bread/init.lua"
      include "tribes/wares/mead/init.lua"
      include "tribes/wares/scrap_iron/init.lua"
      include "tribes/wares/scrap_metal_mixed/init.lua"
      include "tribes/wares/fur/init.lua"
      include "tribes/wares/fur_garment/init.lua"
      include "tribes/wares/fur_garment_old/init.lua"
      include "tribes/wares/fur_garment_studded/init.lua"
      include "tribes/wares/fur_garment_golden/init.lua"
      include "tribes/wares/helmet_golden/init.lua"
      include "tribes/wares/sword_short/init.lua"
      include "tribes/wares/sword_long/init.lua"
      include "tribes/wares/sword_broad/init.lua"
      include "tribes/wares/sword_double/init.lua"
      include "tribes/wares/needles/init.lua"
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
      include "tribes/wares/beer_strong/init.lua"
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
      include "tribes/wares/tabard/init.lua"
      include "tribes/wares/tabard_golden/init.lua"
      include "tribes/wares/reed/init.lua"
      include "tribes/wares/trident_double/init.lua"
      include "tribes/wares/trident_heavy_double/init.lua"
      include "tribes/wares/trident_light/init.lua"
      include "tribes/wares/trident_long/init.lua"
      include "tribes/wares/trident_steel/init.lua"
      include "tribes/wares/water/init.lua"
      include "tribes/wares/wheat/init.lua"
      include "tribes/wares/wine/init.lua"
      include "tribes/wares/wool/init.lua"

   end)

   -- ===================================
   --    Immovables
   -- ===================================

   print_loading_message("┃    Immovables", function()
      egbase:set_loading_message(_("Loading tribes: Immovables (3/15)"))
      include "tribes/immovables/ashes/init.lua"
      include "tribes/immovables/blackrootfield/harvested/init.lua"
      include "tribes/immovables/blackrootfield/medium/init.lua"
      include "tribes/immovables/blackrootfield/ripe/init.lua"
      include "tribes/immovables/blackrootfield/small/init.lua"
      include "tribes/immovables/blackrootfield/tiny/init.lua"
      include "tribes/immovables/cornfield/harvested/init.lua"
      include "tribes/immovables/cornfield/medium/init.lua"
      include "tribes/immovables/cornfield/ripe/init.lua"
      include "tribes/immovables/cornfield/small/init.lua"
      include "tribes/immovables/cornfield/tiny/init.lua"
      include "tribes/immovables/destroyed_building/init.lua"
      include "tribes/immovables/wheatfield/harvested/init.lua"
      include "tribes/immovables/wheatfield/medium/init.lua"
      include "tribes/immovables/wheatfield/ripe/init.lua"
      include "tribes/immovables/wheatfield/small/init.lua"
      include "tribes/immovables/wheatfield/tiny/init.lua"
      include "tribes/immovables/grapevine/medium/init.lua"
      include "tribes/immovables/grapevine/ripe/init.lua"
      include "tribes/immovables/grapevine/small/init.lua"
      include "tribes/immovables/grapevine/tiny/init.lua"
      include "tribes/immovables/pond/growing/init.lua"
      include "tribes/immovables/pond/mature/init.lua"
      include "tribes/immovables/pond/dry/init.lua"
      include "tribes/immovables/reedfield/medium/init.lua"
      include "tribes/immovables/reedfield/ripe/init.lua"
      include "tribes/immovables/reedfield/small/init.lua"
      include "tribes/immovables/reedfield/tiny/init.lua"
      include "tribes/immovables/resi/atlanteans/init.lua"
      include "tribes/immovables/resi/barbarians/init.lua"
      include "tribes/immovables/resi/empire/init.lua"
      include "tribes/immovables/resi/frisians/init.lua"
      include "tribes/immovables/shipconstruction_atlanteans/init.lua"
      include "tribes/immovables/shipconstruction_barbarians/init.lua"
      include "tribes/immovables/shipconstruction_empire/init.lua"
      include "tribes/immovables/shipconstruction_frisians/init.lua"
      include "tribes/immovables/berry_bushes/blueberry/init.lua"
      include "tribes/immovables/berry_bushes/raspberry/init.lua"
      include "tribes/immovables/berry_bushes/currant_red/init.lua"
      include "tribes/immovables/berry_bushes/currant_black/init.lua"
      include "tribes/immovables/berry_bushes/strawberry/init.lua"
      include "tribes/immovables/berry_bushes/sea_buckthorn/init.lua"
      include "tribes/immovables/berry_bushes/desert_hackberry/init.lua"
      include "tribes/immovables/berry_bushes/juniper/init.lua"
      include "tribes/immovables/barleyfield/tiny/init.lua"
      include "tribes/immovables/barleyfield/small/init.lua"
      include "tribes/immovables/barleyfield/medium/init.lua"
      include "tribes/immovables/barleyfield/ripe/init.lua"
      include "tribes/immovables/barleyfield/harvested/init.lua"
   end)

   -- ===================================
   --    Workers, Carriers & Soldiers
   -- ===================================

   print_loading_message("┃    Workers", function()
      egbase:set_loading_message(_("Loading tribes: Atlantean workers (4/15)"))
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
      include "tribes/workers/atlanteans/recruit/init.lua"
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

      egbase:set_loading_message(_("Loading tribes: Barbarian workers (5/15)"))
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
      include "tribes/workers/barbarians/recruit/init.lua"
      include "tribes/workers/barbarians/scout/init.lua"
      include "tribes/workers/barbarians/shipwright/init.lua"
      include "tribes/workers/barbarians/smelter/init.lua"
      include "tribes/workers/barbarians/soldier/init.lua"
      include "tribes/workers/barbarians/stonemason/init.lua"
      include "tribes/workers/barbarians/trainer/init.lua"
      include "tribes/workers/barbarians/weaver/init.lua"

      egbase:set_loading_message(_("Loading tribes: Empire workers (6/15)"))
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
      include "tribes/workers/empire/recruit/init.lua"
      include "tribes/workers/empire/scout/init.lua"
      include "tribes/workers/empire/shepherd/init.lua"
      include "tribes/workers/empire/shipwright/init.lua"
      include "tribes/workers/empire/smelter/init.lua"
      include "tribes/workers/empire/soldier/init.lua"
      include "tribes/workers/empire/stonemason/init.lua"
      include "tribes/workers/empire/toolsmith/init.lua"
      include "tribes/workers/empire/trainer/init.lua"
      include "tribes/workers/empire/vinefarmer/init.lua"
      include "tribes/workers/empire/vintner/init.lua"
      include "tribes/workers/empire/weaponsmith/init.lua"
      include "tribes/workers/empire/weaver/init.lua"

      egbase:set_loading_message(_("Loading tribes: Frisian workers (7/15)"))
      include "tribes/workers/frisians/carrier/init.lua"
      include "tribes/workers/frisians/reindeer/init.lua"
      include "tribes/workers/frisians/builder/init.lua"
      include "tribes/workers/frisians/soldier/init.lua"
      include "tribes/workers/frisians/miner_master/init.lua"
      include "tribes/workers/frisians/miner/init.lua"
      include "tribes/workers/frisians/baker_master/init.lua"
      include "tribes/workers/frisians/baker/init.lua"
      include "tribes/workers/frisians/brewer_master/init.lua"
      include "tribes/workers/frisians/brewer/init.lua"
      include "tribes/workers/frisians/stonemason/init.lua"
      include "tribes/workers/frisians/geologist/init.lua"
      include "tribes/workers/frisians/woodcutter/init.lua"
      include "tribes/workers/frisians/forester/init.lua"
      include "tribes/workers/frisians/reed_farmer/init.lua"
      include "tribes/workers/frisians/berry_farmer/init.lua"
      include "tribes/workers/frisians/farmer/init.lua"
      include "tribes/workers/frisians/blacksmith_master/init.lua"
      include "tribes/workers/frisians/blacksmith/init.lua"
      include "tribes/workers/frisians/smoker/init.lua"
      include "tribes/workers/frisians/landlady/init.lua"
      include "tribes/workers/frisians/brickmaker/init.lua"
      include "tribes/workers/frisians/claydigger/init.lua"
      include "tribes/workers/frisians/charcoal_burner/init.lua"
      include "tribes/workers/frisians/seamstress_master/init.lua"
      include "tribes/workers/frisians/seamstress/init.lua"
      include "tribes/workers/frisians/trainer/init.lua"
      include "tribes/workers/frisians/fruit_collector/init.lua"
      include "tribes/workers/frisians/beekeeper/init.lua"
      include "tribes/workers/frisians/reindeer_breeder/init.lua"
      include "tribes/workers/frisians/fisher/init.lua"
      include "tribes/workers/frisians/hunter/init.lua"
      include "tribes/workers/frisians/smelter/init.lua"
      include "tribes/workers/frisians/shipwright/init.lua"
      include "tribes/workers/frisians/scout/init.lua"
   end)

   -- ===================================
   --    Warehouses
   -- ===================================

   print_loading_message("┃    Warehouses", function()
      egbase:set_loading_message(_("Loading tribes: Warehouses (8/15)"))
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
      include "tribes/buildings/warehouses/frisians/headquarters/init.lua"
      include "tribes/buildings/warehouses/frisians/port/init.lua"
      include "tribes/buildings/warehouses/frisians/warehouse/init.lua"
   end)

   -- ===================================
   --    Productionsites
   -- ===================================

   print_loading_message("┃    Productionsites", function()
      egbase:set_loading_message(_("Loading tribes: Atlantean productionsites (9/15)"))
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
      include "tribes/buildings/productionsites/atlanteans/barracks/init.lua"

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
      egbase:set_loading_message(_("Loading tribes: Barbarian productionsites (10/15)"))
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
      include "tribes/buildings/productionsites/barbarians/ax_workshop/init.lua"
      include "tribes/buildings/productionsites/barbarians/metal_workshop/init.lua"
      include "tribes/buildings/productionsites/barbarians/barracks/init.lua"

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
      egbase:set_loading_message(_("Loading tribes: Empire productionsites (11/15)"))
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
      include "tribes/buildings/productionsites/empire/barracks/init.lua"

      -- Empire mines
      include "tribes/buildings/productionsites/empire/coalmine_deep/init.lua"
      include "tribes/buildings/productionsites/empire/coalmine/init.lua"
      include "tribes/buildings/productionsites/empire/ironmine_deep/init.lua"
      include "tribes/buildings/productionsites/empire/ironmine/init.lua"
      include "tribes/buildings/productionsites/empire/marblemine_deep/init.lua"
      include "tribes/buildings/productionsites/empire/marblemine/init.lua"
      include "tribes/buildings/productionsites/empire/goldmine_deep/init.lua"
      include "tribes/buildings/productionsites/empire/goldmine/init.lua"

      --Frisians mines
      egbase:set_loading_message(_("Loading tribes: Frisian productionsites (12/15)"))
      include "tribes/buildings/productionsites/frisians/coalmine_deep/init.lua"
      include "tribes/buildings/productionsites/frisians/rockmine_deep/init.lua"
      include "tribes/buildings/productionsites/frisians/goldmine_deep/init.lua"
      include "tribes/buildings/productionsites/frisians/ironmine_deep/init.lua"
      include "tribes/buildings/productionsites/frisians/coalmine/init.lua"
      include "tribes/buildings/productionsites/frisians/rockmine/init.lua"
      include "tribes/buildings/productionsites/frisians/goldmine/init.lua"
      include "tribes/buildings/productionsites/frisians/ironmine/init.lua"
      --Frisians big
      include "tribes/buildings/productionsites/frisians/farm/init.lua"
      include "tribes/buildings/productionsites/frisians/reindeer_farm/init.lua"
      --Frisians small
      include "tribes/buildings/productionsites/frisians/woodcutters_house/init.lua"
      include "tribes/buildings/productionsites/frisians/foresters_house/init.lua"
      include "tribes/buildings/productionsites/frisians/quarry/init.lua"
      include "tribes/buildings/productionsites/frisians/scouts_house/init.lua"
      include "tribes/buildings/productionsites/frisians/beekeepers_house/init.lua"
      include "tribes/buildings/productionsites/frisians/reed_farm/init.lua"
      include "tribes/buildings/productionsites/frisians/well/init.lua"
      include "tribes/buildings/productionsites/frisians/aqua_farm/init.lua"
      include "tribes/buildings/productionsites/frisians/hunters_house/init.lua"
      include "tribes/buildings/productionsites/frisians/fishers_house/init.lua"
      include "tribes/buildings/productionsites/frisians/berry_farm/init.lua"
      include "tribes/buildings/productionsites/frisians/collectors_house/init.lua"
      include "tribes/buildings/productionsites/frisians/clay_pit/init.lua"
      --Frisians medium
      include "tribes/buildings/productionsites/frisians/honey_bread_bakery/init.lua"
      include "tribes/buildings/productionsites/frisians/bakery/init.lua"
      include "tribes/buildings/productionsites/frisians/drinking_hall/init.lua"
      include "tribes/buildings/productionsites/frisians/tavern/init.lua"
      include "tribes/buildings/productionsites/frisians/mead_brewery/init.lua"
      include "tribes/buildings/productionsites/frisians/brewery/init.lua"
      include "tribes/buildings/productionsites/frisians/weaving_mill/init.lua"
      include "tribes/buildings/productionsites/frisians/smokery/init.lua"
      include "tribes/buildings/productionsites/frisians/shipyard/init.lua"
      include "tribes/buildings/productionsites/frisians/furnace/init.lua"
      include "tribes/buildings/productionsites/frisians/recycling_center/init.lua"
      include "tribes/buildings/productionsites/frisians/blacksmithy/init.lua"
      include "tribes/buildings/productionsites/frisians/armor_smithy_large/init.lua"
      include "tribes/buildings/productionsites/frisians/armor_smithy_small/init.lua"
      include "tribes/buildings/productionsites/frisians/tailors_shop/init.lua"
      include "tribes/buildings/productionsites/frisians/sewing_room/init.lua"
      include "tribes/buildings/productionsites/frisians/charcoal_kiln/init.lua"
      include "tribes/buildings/productionsites/frisians/brick_kiln/init.lua"
      include "tribes/buildings/productionsites/frisians/barracks/init.lua"
   end)

   -- ===================================
   --    Trainingsites
   -- ===================================

   print_loading_message("┃    Trainingsites", function()
      egbase:set_loading_message(_("Loading tribes: Trainingsites (13/15)"))
      include "tribes/buildings/trainingsites/atlanteans/dungeon/init.lua"
      include "tribes/buildings/trainingsites/atlanteans/labyrinth/init.lua"
      include "tribes/buildings/trainingsites/barbarians/battlearena/init.lua"
      include "tribes/buildings/trainingsites/barbarians/trainingcamp/init.lua"
      include "tribes/buildings/trainingsites/empire/colosseum/init.lua"
      include "tribes/buildings/trainingsites/empire/arena/init.lua"
      include "tribes/buildings/trainingsites/empire/trainingcamp/init.lua"
      include "tribes/buildings/trainingsites/frisians/training_camp/init.lua"
      include "tribes/buildings/trainingsites/frisians/training_arena/init.lua"
   end)

   -- ===================================
   --    Militarysites
   -- ===================================

   print_loading_message("┃    Militarysites", function()
      egbase:set_loading_message(_("Loading tribes: Militarysites (14/15)"))
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

      include "tribes/buildings/militarysites/frisians/wooden_tower_high/init.lua"
      include "tribes/buildings/militarysites/frisians/wooden_tower/init.lua"
      include "tribes/buildings/militarysites/frisians/sentinel/init.lua"
      include "tribes/buildings/militarysites/frisians/outpost/init.lua"
      include "tribes/buildings/militarysites/frisians/tower/init.lua"
      include "tribes/buildings/militarysites/frisians/fortress/init.lua"
   end)

   -- ===================================
   --    Partially Finished Buildings
   -- ===================================

egbase:set_loading_message(_("Loading tribes: Finishing (15/15)"))

   print_loading_message("┃    Partially finished buildings", function()
      include "tribes/buildings/partially_finished/constructionsite/init.lua"
      include "tribes/buildings/partially_finished/dismantlesite/init.lua"
   end)

   -- ===================================
   --    Tribes
   -- ===================================
   print_loading_message("┃    Tribe infos", function()
      include "tribes/atlanteans.lua"
      include "tribes/barbarians.lua"
      include "tribes/empire.lua"
      include "tribes/frisians.lua"
   end)


end)
