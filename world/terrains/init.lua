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
   -- Temperature is in arbitrary units.
   temperature = 100,

   -- Humidity is in percent (1 being very wet).
   humidity = 0.6,

   -- Fertility is in percent (1 being very fertile).
   fertility = 0.7,
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
   temperature = 100,
   humidity = 0.6,
   fertility = 0.65,

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
   temperature = 105,
   humidity = 0.55,
   fertility = 0.8,
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
   temperature = 110,
   humidity = 0.65,
   fertility = 0.75,
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
   temperature = 100,
   humidity = 0.4,
   fertility = 0.4,
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
   temperature = 100,
   humidity = 0.15,
   fertility = 0.15,
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
   temperature = 85,
   humidity = 0.75,
   fertility = 0.5,
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
   temperature = 80,
   humidity = 0.1,
   fertility = 0.1,
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
   temperature = 80,
   humidity = 0.1,
   fertility = 0.1,
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
   temperature = 80,
   humidity = 0.1,
   fertility = 0.1,
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
   temperature = 80,
   humidity = 0.1,
   fertility = 0.1,
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
   temperature = 105,
   humidity = 0.999,
   fertility = 0.1,
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
   temperature = 120,
   humidity = 0.6,
   fertility = 0.2,
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
   temperature = 50,
   humidity = 0.999,
   fertility = 0.001,
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
   humidity = 0.001,
   fertility = 0.001,
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
   temperature = 100,
   humidity = 0.999,
   fertility = 0.001,
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
   temperature = 120,
   humidity = 0.15,
   fertility = 0.9,
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
   temperature = 125,
   humidity = 0.1,
   fertility = 0.999,
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
   temperature = 100,
   humidity = 0.25,
   fertility = 0.8,
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
   temperature = 95,
   humidity = 0.15,
   fertility = 0.85,
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
   temperature = 105,
   humidity = 0.2,
   fertility = 0.9,
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
   temperature = 90,
   humidity = 0.2,
   fertility = 0.8,
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
   temperature = 120,
   humidity = 0.1,
   fertility = 0.25,
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
   temperature = 80,
   humidity = 0.05,
   fertility = 0.2,
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
   temperature = 80,
   humidity = 0.05,
   fertility = 0.2,
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
   temperature = 80,
   humidity = 0.05,
   fertility = 0.2,
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
   temperature = 80,
   humidity = 0.05,
   fertility = 0.2,
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
   temperature = 60,
   humidity = 0.4,
   fertility = 0.2,
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
   humidity = 0.001,
   fertility = 0.001,
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
   humidity = 0.001,
   fertility = 0.001,
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
   temperature = 100,
   humidity = 0.999,
   fertility = 0.001,
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
   temperature = 45,
   humidity = 0.85,
   fertility = 0.5,
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
   temperature = 55,
   humidity = 0.75,
   fertility = 0.45,
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
   temperature = 50,
   humidity = 0.8,
   fertility = 0.4,
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
   temperature = 40,
   humidity = 0.75,
   fertility = 0.4,
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
   temperature = 35,
   humidity = 0.75,
   fertility = 0.3,
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
   temperature = 25,
   humidity = 0.8,
   fertility = 0.1,
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
   temperature = 20,
   humidity = 0.3,
   fertility = 0.05,
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
   temperature = 20,
   humidity = 0.3,
   fertility = 0.05,
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
   temperature = 20,
   humidity = 0.3,
   fertility = 0.05,
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
   temperature = 20,
   humidity = 0.3,
   fertility = 0.05,
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
   temperature = 25,
   humidity = 0.5,
   fertility = 0.1,
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
   temperature = 60,
   humidity = 0.5,
   fertility = 0.1,
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
   temperature = 50,
   humidity = 0.999,
   fertility = 0.001,
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
   temperature = 50,
   humidity = 0.999,
   fertility = 0.001,
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
   temperature = 50,
   humidity = 0.999,
   fertility = 0.001,
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
   temperature = 168,
   humidity = 0.001,
   fertility = 0.1,
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
   temperature = 172,
   humidity = 0.2,
   fertility = 0.2,
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
   temperature = 155,
   humidity = 0.5,
   fertility = 0.5,
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
   temperature = 160,
   humidity = 0.6,
   fertility = 0.6,
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
   temperature = 145,
   humidity = 0.5,
   fertility = 0.5,
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
   temperature = 140,
   humidity = 0.4,
   fertility = 0.4,
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
   temperature = 130,
   humidity = 0.05,
   fertility = 0.05,
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
   temperature = 130,
   humidity = 0.05,
   fertility = 0.05,
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
   temperature = 130,
   humidity = 0.05,
   fertility = 0.05,
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
   temperature = 130,
   humidity = 0.05,
   fertility = 0.05,
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
   temperature = 167,
   humidity = 0.001,
   fertility = 0.001,
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
   temperature = 168,
   humidity = 0.001,
   fertility = 0.001,
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
   temperature = 178,
   humidity = 0.001,
   fertility = 0.001,
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
   temperature = 179,
   humidity = 0.5,
   fertility = 0.1,
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
   temperature = 150,
   humidity = 0.999,
   fertility = 0.001,
}
