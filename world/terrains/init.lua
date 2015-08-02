-- Categories for the editor.
world:new_editor_terrain_category{
   name = "green",
   descname = _ "Summer",
   picture = "world/pics/editor_terrain_category_green.png",
}
world:new_editor_terrain_category{
   name = "wasteland",
   descname = _ "Wasteland",
   picture = "world/pics/editor_terrain_category_wasteland.png",
}
world:new_editor_terrain_category{
   name = "winter",
   descname = _ "Winter",
   picture = "world/pics/editor_terrain_category_winter.png",
}
world:new_editor_terrain_category{
   name = "desert",
   descname = _ "Desert",
   picture = "world/pics/editor_terrain_category_desert.png",
}

------------------------
--  Former greenland  --
------------------------

pics_dir = path.dirname(__file__) .. "pics/"
world:new_terrain_type{
   -- The internal name of this terrain.
   name = "wiese1",

   -- The name that will be used in UI and translated.
   descname = _ "Meadow",

   -- The category for sorting this into menus in the editor.
   editor_category = "green",

   -- Type of terrain. Describes if the terrain is walkable, swimmable, if
   -- mines or buildings can be build on it, if flags can be build on it and so
   -- on.
   --
   -- The following properties are available:
   -- "green": Allows building of normal buildings and roads
   -- "mountain": Allows building of mines and roads
   -- "dry": Allows building of roads only. Trees don't like this terrain.
   -- "water": Nothing can be built here, but ships and aquatic animals can pass
   -- "dead": Nothing can be built here, and nothing can walk on it, and nothing will grow.
   -- "impassable": Nothing can be built here, and nothing can walk on it
   is = "green",

   -- The list resources that can be found in this terrain.
   valid_resources = {"water"},

   -- The resources that is always in this terrain (if not overwritten by the
   -- map maker through the editor) and the amount.
   default_resource = "water",
   default_resource_amount = 10,

   -- The images used for this terrain.
   textures = { pics_dir .. "green/wiese1_00.png" },

   -- This describes the z layer of the terrain when rendered next to another
   -- one and blending slightly over it to hide the triangles.
   dither_layer = 340,

   -- Terrain affinity constants. This is used to model how well plants grow on this terrain.
   -- Temperature are in Kelvin.
   temperature = 300.04278317924405,

   -- Humidity is in percent (1 being very wet).
   humidity = 0.55340250282816961,

   -- Fertility is in percent (1 being very fertile).
   fertility = 0.53191138330107279,
}

world:new_terrain_type{
   name = "wiese2",
   descname = _ "Meadow",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "green/wiese2_00.png" },
   dither_layer = 350,
   temperature = 295.65369623095955,
   humidity = 0.68188133706990839,
   fertility = 0.42867491634505633,

}

world:new_terrain_type{
   name = "wiese3",
   descname = _ "Meadow",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "green/wiese3_00.png" },
   dither_layer = 350,
   temperature = 294.08241457107198,
   humidity = 0.67113553507660639,
   fertility = 0.41316511937935013,
}

world:new_terrain_type{
   name = "wiese4",
   descname = _ "Meadow",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "green/wiese4_00.png" },
   dither_layer = 350,
   temperature = 288.58389722412585,
   humidity = 0.67472017740529411,
   fertility = 0.41570129413077317,
}

world:new_terrain_type{
   name = "steppe",
   descname = _ "Steppe",
   editor_category = "green",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 5,
   textures = { pics_dir .. "green/steppe_00.png" },
   dither_layer = 330,
   temperature = 295.73278633026462,
   humidity = 0.89597247873747632,
   fertility = 0.15303469813105364,
}

world:new_terrain_type{
   name = "steppe_kahl",
   descname = _ "Barren Steppe",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 4,
   textures = { pics_dir .. "green/steppe_kahl_00.png" },
   dither_layer = 320,
   temperature = 308.05534629938688,
   humidity = 0.47595742923029816,
   fertility = 0.98842836360535302,
}

world:new_terrain_type{
   name = "bergwiese",
   descname = _ "Mountain Meadow",
   editor_category = "green",
   is = "green",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/bergwiese_00.png" },
   dither_layer = 160,
   temperature = 289.05001901926897,
   humidity = 0.51128431681792141,
   fertility = 0.61550531961182953,
}

world:new_terrain_type{
   name = "berg1",
   descname = _ "Mountain",
   editor_category = "green",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/berg1_00.png" },
   dither_layer = 70,
   temperature = 286.5853971152747,
   humidity = 0.392290548702316,
   fertility = 0.90073346602648985,
}

world:new_terrain_type{
   name = "berg2",
   descname = _ "Mountain",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/berg2_00.png" },
   dither_layer = 70,
   temperature = 286.22963024142905,
   humidity = 0.19387388774216724,
   fertility = 0.67416621022233836,
}

world:new_terrain_type{
   name = "berg3",
   descname = _ "Mountain",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/berg3_00.png" },
   dither_layer = 70,
   temperature = 281.41196619670609,
   humidity = 0.19972412128499525,
   fertility = 0.67259046087670449,
}

world:new_terrain_type{
   name = "berg4",
   descname = _ "Mountain",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/berg4_00.png" },
   dither_layer = 70,
   temperature = 281.41200738401443,
   humidity = 0.19973909047983246,
   fertility = 0.67261898072511717,
}

world:new_terrain_type{
   name = "sumpf",
   descname = _ "Swamp",
   editor_category = "green",
   is = "impassable",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 20,
   textures = path.list_directory(pics_dir .. "green", "sumpf_\\d+.png"),
   dither_layer = 370,
   fps = 14,
   temperature = 289.89394318611949,
   humidity = 0.47310737388281437,
   fertility = 0.25499952877430737,
}

world:new_terrain_type{
   name = "strand",
   descname = _ "Beach",
   editor_category = "green",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/strand_00.png" },
   dither_layer = 60,
   temperature = 295.31687025892853,
   humidity = 0.152564445518208,
   fertility = 0.58492011270054045,
}

world:new_terrain_type{
   name = "schnee",
   descname = _ "Snow",
   editor_category = "green",
   is = "dead",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/schnee_00.png" },
   dither_layer = 220,
   temperature = 273.0,
   humidity = 0.99999999999999989,
   fertility = 9.9999999999999998e-17,
}

world:new_terrain_type{
   name = "lava",
   descname = _ "Lava",
   editor_category = "green",
   is = "dead",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_directory(pics_dir .. "green", "lava_\\d+.png"),
   dither_layer = 30,
   fps = 4,
   temperature = 1273.0,
   humidity = 9.9999999999999998e-17,
   fertility = 0.98999999999999999,
}

world:new_terrain_type{
   name = "wasser",
   descname = _ "Water",
   editor_category = "green",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.list_directory(pics_dir .. "green", "wasser_\\d+.png"),
   dither_layer = 180,
   fps = 14,
   temperature = 293.0,
   humidity = 0.99999999999999989,
   fertility = 0.25,
}
------------------------
--  Former blackland  --
------------------------

world:new_terrain_type{
   name = "ashes",
   descname = _ "Ashes",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 5,
   textures = { pics_dir .. "wasteland/ashes_00.png" },
   dither_layer = 400,
   temperature = 308.50931307699597,
   humidity = 0.31736633817980997,
   fertility = 0.25631387504753977,
}

world:new_terrain_type{
   name = "ashes2",
   descname = _ "Ashes",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 4,
   textures = { pics_dir .. "wasteland/ashes2_00.png" },
   dither_layer = 410,
   temperature = 306.24553497496134,
   humidity = 0.31400675041069881,
   fertility = 0.22898571164372822,
}

world:new_terrain_type{
   name = "hardground1",
   descname = _ "Hard Ground",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground1_00.png" },
   dither_layer = 420,
   temperature = 309.19891895863259,
   humidity = 0.33690034399837648,
   fertility = 0.33276290074784548,
}

world:new_terrain_type{
   name = "hardground2",
   descname = _ "Hard Ground",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground2_00.png" },
   dither_layer = 370,
   temperature = 309.87872943866665,
   humidity = 0.31851348617841185,
   fertility = 0.30859538499363437,
}

world:new_terrain_type{
   name = "hardground3",
   descname = _ "Hard Ground",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground3_00.png" },
   dither_layer = 380,
   temperature = 311.4313401428405,
   humidity = 0.31789599269051261,
   fertility = 0.30902388622783256,
}

world:new_terrain_type{
   name = "hardground4",
   descname = _ "Hard Ground",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground4_00.png" },
   dither_layer = 390,
   temperature = 312.2828410320538,
   humidity = 0.31525755144085948,
   fertility = 0.31054473740033578,
}

world:new_terrain_type{
   name = "hardlava",
   descname = _ "Igneous Rocks",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/hardlava_00.png" },
   dither_layer = 360,
   temperature = 319.00977864170756,
   humidity = 0.35447880224872441,
   fertility = 0.23705760252500771,
}

world:new_terrain_type{
   name = "wasteland_mountain1",
   descname = _ "Mountain",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain1_00.png" },
   dither_layer = 90,
   temperature = 305.47980607787952,
   humidity = 0.43741264859131379,
   fertility = 0.4942960101430538,
}

world:new_terrain_type{
   name = "wasteland_mountain2",
   descname = _ "Mountain",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain2_00.png" },
   dither_layer = 90,
   temperature = 308.11414726442104,
   humidity = 0.43590895148164582,
   fertility = 0.50211854383693189,
}

world:new_terrain_type{
   name = "wasteland_mountain3",
   descname = _ "Mountain",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain3_00.png" },
   dither_layer = 90,
   temperature = 309.2313908535491,
   humidity = 0.43430953848524012,
   fertility = 0.50532910869111358,
}

world:new_terrain_type{
   name = "wasteland_mountain4",
   descname = _ "Mountain",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain4_00.png" },
   dither_layer = 80,
   temperature = 309.23137655560333,
   humidity = 0.43430890944898498,
   fertility = 0.50532858192033259,
}

world:new_terrain_type{
   name = "wasteland_beach",
   descname = _ "Beach",
   editor_category = "wasteland",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/strand_00.png" },
   dither_layer = 50,
   temperature = 302.78152301395431,
   humidity = 0.25378089694315337,
   fertility = 0.41946731913025403,
}

world:new_terrain_type{
   name = "lava-stone1",
   descname = _ "Lava Rocks",
   editor_category = "wasteland",
   is = "dead",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_directory(pics_dir .. "wasteland", "lava-stone1_\\d+.png"),
   dither_layer = 20,
   fps = 7,
   temperature = 1273.0,
   humidity = 9.9999999999999998e-17,
   fertility = 0.98999999999999999,
}

world:new_terrain_type{
   name = "lava-stone2",
   descname = _ "Lava Rocks",
   editor_category = "wasteland",
   is = "dead",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_directory(pics_dir .. "wasteland", "lava-stone2_\\d+.png"),
   dither_layer = 10,
   fps = 7,
   temperature = 1273.0,
   humidity = 9.9999999999999998e-17,
   fertility = 0.98999999999999999,
}

world:new_terrain_type{
   name = "wasteland_water",
   descname = _ "Water",
   editor_category = "wasteland",
   is = "water",
   valid_resources = { "fish" },
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.list_directory(pics_dir .. "wasteland", "water_\\d+.png"),
   dither_layer = 170,
   fps = 14,
   temperature = 313.0,
   humidity = 0.99999999999999989,
   fertility = 0.80000000000000004,
}

-------------------------
--  Former Winterland  --
-------------------------

world:new_terrain_type{
   name = "tundra",
   descname = _ "Tundra",
   editor_category = "winter",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra_00.png" },
   dither_layer = 230,
   temperature = 285.15816824963719,
   humidity = 0.77781437710174905,
   fertility = 0.46117587943158045,
}

world:new_terrain_type{
   name = "tundra2",
   descname = _ "Tundra",
   editor_category = "winter",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra2_00.png" },
   dither_layer = 240,
   temperature = 299.65831265959122,
   humidity = 0.71243181397420374,
   fertility = 0.38729356301371143,
}

world:new_terrain_type{
   name = "tundra3",
   descname = _ "Tundra",
   editor_category = "winter",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra3_00.png" },
   dither_layer = 240,
   temperature = 291.97891709336449,
   humidity = 0.71392329660321152,
   fertility = 0.39949724244349072,
}

world:new_terrain_type{
   name = "tundra_taiga",
   descname = _ "Tundra Taiga",
   editor_category = "winter",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra_taiga_00.png" },
   dither_layer = 230,
   temperature = 290.59652759132916,
   humidity = 0.52019070699260228,
   fertility = 0.25232407403911222,
}

world:new_terrain_type{
   name = "taiga",
   descname = _ "Taiga",
   editor_category = "winter",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/taiga_00.png" },
   dither_layer = 250,
   temperature = 285.35526221552516,
   humidity = 0.48486236876174976,
   fertility = 0.16382912093990756,
}

world:new_terrain_type{
   name = "snow",
   descname = _ "Snow",
   editor_category = "winter",
   is = "green",
   valid_resources = { "water" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/snow_00.png" },
   dither_layer = 250,
   temperature = 255.71533125102573,
   humidity = 0.67569418221390831,
   fertility = 0.4755944578602016,
}

world:new_terrain_type{
   name = "winter_mountain1",
   descname = _ "Mountain",
   editor_category = "winter",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain1_00.png" },
   dither_layer = 110,
   temperature = 234.3236440452477,
   humidity = 0.353894295696628,
   fertility = 0.64095166379264268,
}

world:new_terrain_type{
   name = "winter_mountain2",
   descname = _ "Mountain",
   editor_category = "winter",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain2_00.png" },
   dither_layer = 110,
   temperature = 234.32036692949509,
   humidity = 0.35388395206559686,
   fertility = 0.64097548376370639,
}

world:new_terrain_type{
   name = "winter_mountain3",
   descname = _ "Mountain",
   editor_category = "winter",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain3_00.png" },
   dither_layer = 100,
   temperature = 234.32370923956316,
   humidity = 0.35386704652440848,
   fertility = 0.64096302130057681,
}

world:new_terrain_type{
   name = "winter_mountain4",
   descname = _ "Mountain",
   editor_category = "winter",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain4_00.png" },
   dither_layer = 100,
   temperature = 234.32284033138464,
   humidity = 0.35387628237099111,
   fertility = 0.64096295347399923,
}

world:new_terrain_type{
   name = "ice",
   descname = _ "Ice",
   editor_category = "winter",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/ice_00.png" },
   dither_layer = 260,
   temperature = 241.18389899353673,
   humidity = 0.41922543451566474,
   fertility = 0.41963058218715021,
}

world:new_terrain_type{
   name = "winter_beach",
   descname = _ "Beach",
   editor_category = "winter",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/strand_00.png" },
   dither_layer = 40,
   temperature = 243.04482604455663,
   humidity = 0.52048324144504288,
   fertility = 0.35517827723739792,
}

world:new_terrain_type{
   name = "ice_floes",
   descname = _ "Ice Floes",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_directory(pics_dir .. "winter", "ice_flows_\\d+.png"),
   dither_layer = 210,
   fps = 5,
   temperature = 265.80526108661786,
   humidity = 0.13459817136515448,
   fertility = 0.29392443481224101,
}

world:new_terrain_type{
   name = "ice_floes2",
   descname = _ "Ice Floes",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_directory(pics_dir .. "winter", "ice_flows2_\\d+.png"),
   dither_layer = 210,
   fps = 5,
   temperature = 265.80528666077464,
   humidity = 0.13462161525263067,
   fertility = 0.29399226894281316,
}

world:new_terrain_type{
   name = "winter_water",
   descname = _ "Water",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.list_directory(pics_dir .. "winter", "water_\\d+.png"),
   dither_layer = 190,
   fps = 8,
   temperature = 273.0,
   humidity = 0.99999999999999989,
   fertility = 0.40000000000000002,
}

---------------------
--  Former Desert  --
---------------------

world:new_terrain_type{
   name = "desert4",
   descname = _ "Desert",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 2,
   textures = { pics_dir .. "desert/desert4_00.png" },
   dither_layer = 270,
   temperature = 318.95927963394809,
   humidity = 0.19850680766512677,
   fertility = 0.26285391621490406,
}

world:new_terrain_type{
   name = "drysoil",
   descname = _ "Dry Soil",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 4,
   textures = { pics_dir .. "desert/drysoil_00.png" },
   dither_layer = 300,
   temperature = 322.57200571542654,
   humidity = 0.33221932657651621,
   fertility = 0.24996290818869515,
}

world:new_terrain_type{
   name = "desert_steppe",
   descname = _ "Steppe",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 5,
   textures = { pics_dir .. "desert/steppe_00.png" },
   dither_layer = 360,
   temperature = 307.66693414903386,
   humidity = 0.50785131521278204,
   fertility = 0.40543024435718211,
}

world:new_terrain_type{
   name = "meadow",
   descname = _ "Meadow",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "desert/meadow_00.png" },
   dither_layer = 310,
   temperature = 305.07841308001815,
   humidity = 0.66409487876828999,
   fertility = 0.28982229496468942,
}

world:new_terrain_type{
   name = "mountainmeadow",
   descname = _ "Mountain Meadow",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountainmeadow_00.png" },
   dither_layer = 150,
   temperature = 289.36932830392055,
   humidity = 0.85111697807847664,
   fertility = 0.39411986912974672,
}

world:new_terrain_type{
   name = "highmountainmeadow",
   descname = _ "High Mountain Meadow",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/highmountainmeadow_00.png" },
   dither_layer = 150,
   temperature = 295.28805163693715,
   humidity = 0.96636799120338213,
   fertility = 0.58747114224515407,
}

world:new_terrain_type{
   name = "mountain1",
   descname = _ "Mountain",
   editor_category = "desert",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain1_00.png" },
   dither_layer = 120,
   temperature = 303.43073881374357,
   humidity = 0.3691356727889481,
   fertility = 0.9955591737548094,
}

world:new_terrain_type{
   name = "mountain2",
   descname = _ "Mountain",
   editor_category = "desert",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain2_00.png" },
   dither_layer = 120,
   temperature = 303.42972829646413,
   humidity = 0.36724469817275668,
   fertility = 0.99483988816595148,
}

world:new_terrain_type{
   name = "mountain3",
   descname = _ "Mountain",
   editor_category = "desert",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain3_00.png" },
   dither_layer = 130,
   temperature = 303.42994321508786,
   humidity = 0.36911137784238873,
   fertility = 0.99554973691737925,
}

world:new_terrain_type{
   name = "mountain4",
   descname = _ "Mountain",
   editor_category = "desert",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain4_00.png" },
   dither_layer = 140,
   temperature = 303.42934087733249,
   humidity = 0.36877037568728566,
   fertility = 0.99539803013386874,
}

world:new_terrain_type{
   name = "desert1",
   descname = _ "Desert",
   editor_category = "desert",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert1_00.png" },
   dither_layer = 290,
   temperature = 317.94592510628115,
   humidity = 0.026741512333483181,
   fertility = 0.40660860612661509,
}

world:new_terrain_type{
   name = "desert2",
   descname = _ "Desert",
   editor_category = "desert",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert2_00.png" },
   dither_layer = 280,
   temperature = 318.08427760835616,
   humidity = 0.014328787840514678,
   fertility = 0.22973365655463399,
}

world:new_terrain_type{
   name = "desert3",
   descname = _ "Desert",
   editor_category = "desert",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert3_00.png" },
   dither_layer = 280,
   temperature = 328.1229356910701,
   humidity = 0.073760305337428986,
   fertility = 0.028419096361354063,
}

world:new_terrain_type{
   name = "desert_beach",
   descname = _ "Beach",
   editor_category = "desert",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/beach_00.png" },
   dither_layer = 60,
   temperature = 329.25806053618618,
   humidity = 0.45713638428482906,
   fertility = 0.067762679294860309,
}

world:new_terrain_type{
   name = "desert_water",
   descname = _ "Water",
   editor_category = "desert",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.list_directory(pics_dir .. "desert", "wasser_\\d+.png"),
   dither_layer = 200,
   fps = 5,
   temperature = 308.0,
   humidity = 0.99999999999999989,
   fertility = 0.98999999999999999,
}
