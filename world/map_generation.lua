-- This file is only used by the random map generator. It is not part of the
-- world definition (i.e. it is not sourced by world/init.lua) and is only
-- loaded when the random map generator is initialized in the Editor.


------------------------
--  Former greenland  --
------------------------


greenland = {
   heights = {
      ocean = 10,
      shelf = 10,
      shallow = 10,
      coast = 11,
      upperland = 14,
      mountainfoot = 15,
      mountain = 16,
      snow = 39,
      summit = 45,
   },
   areas = {
      water = {
         {
            name = "ocean",
            weight = 1,
            ocean_terrains = { "wasser" },
            shelf_terrains = { "wasser" },
            shallow_terrains = { "wasser" },
         }
      },
      land = {
         {
            name = "beach",
            weight = 2,
            coast_terrains = { "strand" },
            land_terrains = { "wiese2", "wiese3" },
            upper_terrains = { "wiese2", "wiese3" },
         },
         {
            name = "meadow",
            weight = 1,
            coast_terrains = { "strand" },
            land_terrains = { "wiese1" },
            upper_terrains = { "wiese2", "wiese3" },
         }
      },
      wasteland = {
         {
            name = "swamp",
            weight = 1,
            inner_terrains = { "sumpf" },
            outer_terrains = { "sumpf" },
         },
         {
            name = "tundra",
            inner_terrains = { "steppe_kahl" },
            weight = 1,
            outer_terrains = { "steppe" },
         }
      },
      mountains = {
         {
            name = "mountains",
            weight = 1,
            mountainfoot_terrains = { "bergwiese" },
            mountain_terrains = { "berg1", "berg2", "berg3", "berg4" },
            snow_terrains = { "schnee" },
         }
      }
   }, -- areas
   bob_categories = {
      {
         name = "trees",
         immovables = {
            "aspen_summer_old", "oak_summer_old", "spruce_summer_old",
            "alder_summer_old", "birch_summer_old", "beech_summer_old",
            "larch_summer_old", "rowan_summer_old"
         },
         critters = { "brownbear", "chamois", "stag", "elk", "lynx", "reindeer", "wolf" },
      },
      {
         name = "bushes",
         immovables = { "bush1", "bush2", "bush3", "bush4", "bush5" },
         critters = { "wisent", "wolf", "bunny", "elk", "fox", "lynx", "wolf", "marten", "sheep", "sheep" },
      },
      {
         name = "grasses",
         immovables = { "grass1", "grass2", "grass3" },
         critters = { "bunny", "elk", "fox", "lynx", "wolf", "marten", "sheep", "sheep" },
      },
      {
         name = "standing_stones",
         immovables = {
            "standing_stone1_summer", "standing_stone2_summer",
            "standing_stone3_summer", "standing_stone4_summer",
            "standing_stone5_summer", "standing_stone6", "standing_stone7"
         },
         critters = {},
      },
      {
         name = "stones",
         immovables = {
            "greenland_stones1", "greenland_stones2", "greenland_stones3",
            "greenland_stones4", "greenland_stones5", "greenland_stones5",
            "greenland_stones6"
         },
         critters = {},
      },
      {
         name = "wasteland_doodads",
         immovables = {
            "skeleton1", "skeleton2", "skeleton3", "cactus1", "cactus1",
            "cactus2", "cactus2", "fallentree", "deadtree1", "deadtree2",
            "deadtree3", "deadtree4"
         },
         critters = {},
      }
   }, -- bob_categories
   land_resources = {
      {
         name = "forest",
         weight = 6,
         immovable_density = 100,
         critter_density = 20,
         land_coast_bobs = "bushes",
         land_inner_bobs = "trees",
         land_upper_bobs = "bushes",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "bushes",
      },
      {
         name = "bushland",
         weight = 2,
         immovable_density = 50,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "bushes",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "meadow",
         weight = 2,
         immovable_density = 10,
         critter_density = 20,
         land_coast_bobs = "grasses",
         land_inner_bobs = "grasses",
         land_upper_bobs = "grasses",
         wasteland_inner_bobs = "wasteland_doodads",
         wasteland_outer_bobs = "grasses",
      },
      {
         name = "stone",
         weight = 4,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "stones",
         land_upper_bobs = "",
         wasteland_inner_bobs = "stones",
         wasteland_outer_bobs = "stones",
      },
      {
         name = "standing_stones",
         weight = 2,
         immovable_density = 100,
         critter_density = 0,
         land_coast_bobs = "",
         land_inner_bobs = "standing_stones",
         land_upper_bobs = "",
         wasteland_inner_bobs = "standing_stones",
         wasteland_outer_bobs = "standing_stones",
      }
   } -- land resources
}

-- NOCOM(#sirver): temporary
return greenland
------------------------
--  Former blackland  --
------------------------


-- [water]
            -- NOCOM(#sirver): terrains have been renamed
-- ocean_terrains=water
-- shelf_terrains=water
-- shallow_terrains=water


-- [areas]
-- water=water
-- land=hardground1,hardground2,ashes
-- wasteland=wasteland1,wasteland2
-- mountains=mountains


-- [hardground1]
-- weight=2

-- coast_terrains=strand
-- land_terrains=hardground1,hardground2
-- upper_terrains=hardground1,hardground2


-- [hardground2]
-- weight=2

-- coast_terrains=strand
-- land_terrains=hardground3,hardground4
-- upper_terrains=hardground3,hardground4


-- [ashes]
-- weight=1

-- coast_terrains=ashes2
-- land_terrains=ashes
-- upper_terrains=ashes


-- [mountains]
-- mountainfoot_terrains=hardlava
-- mountain_terrains=mountain1,mountain2,mountain3,mountain4
-- snow_terrains=lava,lava-stone1,lava-stone2


-- [wasteland1]
-- weight=1

-- inner_terrains=lava,lava-stone1
-- outer_terrains=hardlava,lava-stone2


-- [wasteland2]
-- weight=2

-- inner_terrains=hardlava,lava-stone2
-- outer_terrains=hardlava,lava-stone2



-- [bobs]
-- land_resources=forest,bushland,meadow,stone,standing_stones
-- categories=trees,bushes,grasses,stones,standing_stones,wasteland_doodads

-- [forest]
-- weight=6
-- immovable_density=40
-- critter_density=20
-- land_coast_bobs=bushes
-- land_inner_bobs=trees
-- land_upper_bobs=bushes
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=bushes

-- [bushland]
-- weight=2
-- immovable_density=50
-- critter_density=20
-- land_coast_bobs=grasses
-- land_inner_bobs=bushes
-- land_upper_bobs=grasses
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=grasses

-- [meadow]
-- weight=2
-- immovable_density=10
-- critter_density=20
-- land_coast_bobs=grasses
-- land_inner_bobs=grasses
-- land_upper_bobs=grasses
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=grasses

-- [stone]
-- weight=4
-- immovable_density=100
-- critter_density=0
-- land_coast_bobs=
-- land_inner_bobs=stones
-- land_upper_bobs=
-- wasteland_inner_bobs=stones
-- wasteland_outer_bobs=stones

-- [standing_stones]
-- weight=2
-- immovable_density=100
-- critter_density=0
-- land_coast_bobs=
-- land_inner_bobs=standing_stones
-- land_upper_bobs=
-- wasteland_inner_bobs=standing_stones
-- wasteland_outer_bobs=standing_stones

-- [trees]
-- immovables=tree1,tree2,tree3,tree4,tree5,tree6,tree7,tree8
-- critters=deer,reindeer

-- [bushes]
-- immovables=bush1,bush2,bush3,bush4,bush5
-- critters=bunny,fox,sheep,sheep

-- [grasses]
-- immovables=grass1,grass2,grass3
-- critters=bunny,fox,sheep,sheep

-- [standing_stones]
-- immovables=sstones1,sstones2,sstones3,sstones4,sstones5,sstones6,sstones7
-- critters=

-- [stones]
-- immovables=stones1,stones2,stones3,stones4,stones5,stones5,stones6,stones6,stones6
-- critters=

-- [wasteland_doodads]
-- immovables=skeleton1,skeleton2,skeleton3,cactus1,cactus1,cactus2,cactus2,deadtree1,deadtree2,deadtree3,deadtree4
-- critters=

-- ---------------------
-- --  Former desert  --
-- ---------------------



-- [heights]
-- ocean=10
-- shelf=10
-- shallow=10
-- coast=11
-- upperland=14
-- mountainfoot=15
-- mountain=16
-- snow=39
-- summit=45

-- [areas]
-- water=water,water_iceflows,water_ice
-- land=land
-- wasteland=wasteland
-- mountains=mountains


-- [water]
-- ocean_terrains=wasser
-- shelf_terrains=wasser
-- shallow_terrains=wasser

-- [desert]
-- inner_terrains=desert1,desert2,desert3,desert4
-- outer_terrains=desert1,desert2,desert3,desert4

-- [mountains]
-- mountainfoot_terrains=mountainmeadow
-- mountain_terrains=mountain1,mountain2,mountain3,mountain4
-- snow_terrains=mountain1,mountain2,mountain3,mountain4

-- [meadow_land]
-- coast_terrains=beach
-- land_terrains=meadow
-- upper_terrains=mountainmeadow




-- [bobs]
-- land_resources=forest,bushland,meadow,stone,standing_stones
-- categories=trees,bushes,grasses,stones,standing_stones,wasteland_doodads

-- [forest]
-- weight=6
-- immovable_density=40
-- critter_density=20
-- land_coast_bobs=bushes
-- land_inner_bobs=trees
-- land_upper_bobs=bushes
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=bushes

-- [bushland]
-- weight=2
-- immovable_density=50
-- critter_density=20
-- land_coast_bobs=grasses
-- land_inner_bobs=bushes
-- land_upper_bobs=grasses
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=grasses

-- [meadow]
-- weight=2
-- immovable_density=10
-- critter_density=20
-- land_coast_bobs=grasses
-- land_inner_bobs=grasses
-- land_upper_bobs=grasses
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=grasses

-- [stone]
-- weight=4
-- immovable_density=100
-- critter_density=0
-- land_coast_bobs=
-- land_inner_bobs=stones
-- land_upper_bobs=
-- wasteland_inner_bobs=stones
-- wasteland_outer_bobs=stones

-- [standing_stones]
-- weight=2
-- immovable_density=100
-- critter_density=0
-- land_coast_bobs=
-- land_inner_bobs=standing_stones
-- land_upper_bobs=
-- wasteland_inner_bobs=standing_stones
-- wasteland_outer_bobs=standing_stones

-- [trees]
-- immovables=tree1,tree2,tree3,tree4,tree5,tree6,tree7,tree8
-- critters=deer

-- [bushes]
-- immovables=bush1,bush4,bush5
-- critters=bunny,fox,sheep,sheep

-- [grasses]
-- immovables=grass1,grass2,grass3,grass1,grass2,grass3,ruin1,ruin2,ruin3,ruin4,ruin5
-- critters=bunny,fox,sheep,sheep

-- [standing_stones]
-- immovables=sstones1,sstones2,sstones3,sstones4,sstones5,sstones6,sstones7
-- critters=

-- [stones]
-- immovables=stones1,stones2,stones3,stones4,stones5,stones5,stones6,stones6,stones6
-- critters=

-- [wasteland_doodads]
-- immovables=skeleton1,skeleton2,skeleton3,cactus1,cactus1,cactus2,cactus2,deadtree1,deadtree2,deadtree3,deadtree4
-- critters=

-- -------------------------
-- --  Former winterland  --
-- -------------------------


-- [areas]
-- water=water
-- land=meadow_land
-- wasteland=desert
-- mountains=mountains


-- [water]
-- ocean_terrains=water
-- shelf_terrains=water
-- shallow_terrains=water

-- [water_iceflows]
-- ocean_terrains=ice_flows,ice_flows2
-- shelf_terrains=ice_flows,ice_flows2
-- shallow_terrains=ice_flows,ice_flows2

-- [water_ice]
-- ocean_terrains=ice
-- shelf_terrains=ice
-- shallow_terrains=ice

-- [land]
-- coast_terrains=strand
-- land_terrains=tundra,tundra2,tundra3
-- upper_terrains=tundra,tundra2,tundra3

-- [mountains]
-- mountainfoot_terrains=tundra_taiga
-- mountain_terrains=mountain1,mountain2,mountain3,mountain4
-- snow_terrains=snow

-- [wasteland]
-- inner_terrains=tundra_taiga
-- outer_terrains=tundra



-- [bobs]
-- land_resources=forest,bushland,meadow,stone,standing_stones
-- categories=trees,bushes,grasses,stones,standing_stones,wasteland_doodads

-- [forest]
-- weight=6
-- immovable_density=100
-- critter_density=20
-- land_coast_bobs=bushes
-- land_inner_bobs=trees
-- land_upper_bobs=bushes
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=bushes

-- [bushland]
-- weight=2
-- immovable_density=50
-- critter_density=20
-- land_coast_bobs=grasses
-- land_inner_bobs=bushes
-- land_upper_bobs=grasses
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=grasses

-- [meadow]
-- weight=2
-- immovable_density=10
-- critter_density=20
-- land_coast_bobs=grasses
-- land_inner_bobs=grasses
-- land_upper_bobs=grasses
-- wasteland_inner_bobs=wasteland_doodads
-- wasteland_outer_bobs=grasses

-- [stone]
-- weight=4
-- immovable_density=100
-- critter_density=0
-- land_coast_bobs=
-- land_inner_bobs=stones
-- land_upper_bobs=
-- wasteland_inner_bobs=stones
-- wasteland_outer_bobs=stones

-- [standing_stones]
-- weight=2
-- immovable_density=100
-- critter_density=0
-- land_coast_bobs=
-- land_inner_bobs=standing_stones
-- land_upper_bobs=
-- wasteland_inner_bobs=standing_stones
-- wasteland_outer_bobs=standing_stones

-- [trees]
-- immovables=tree1,tree2,tree3,tree4,tree5,tree6,tree7,tree8
-- critters=deer,elk,lynx,reindeer,wolf

-- [bushes]
-- immovables=bush1,bush2,bush3,bush4,bush5
-- critters=wisent,wolf,bunny,elk,fox,lynx,wolf,marten,sheep,sheep

-- [grasses]
-- immovables=grass1,grass2,grass3
-- critters=bunny,elk,fox,lynx,wolf,marten,sheep,sheep

-- [standing_stones]
-- immovables=sstones1,sstones2,sstones3,sstones4,sstones5,sstones6,sstones7
-- critters=

-- [stones]
-- immovables=stones1,stones2,stones3,stones4,stones5,stones5,stones6,stones6,stones6
-- critters=

-- [wasteland_doodads]
-- immovables=skeleton1,skeleton2,skeleton3,deadtree1,deadtree2,deadtree3,deadtree4
-- critters=
