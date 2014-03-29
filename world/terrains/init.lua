------------------------
--  Former greenland  --
------------------------

-- NOCOM(#sirver): this is suboptimal
-- Order does matter here, since this file should stay compatible
-- with S2
--

-- NOCOM(#sirver): no need to keep the german names now - translate to english.

pics_dir = path.dirname(__file__) .. "pics/"
world:new_terrain_type{
   name = "wiese1",
   descname = _ "Meadow 1",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "green/wiese1_00.png" },
   dither_layer = 40,
}

world:new_terrain_type{
   name = "wiese2",
   descname = _ "Meadow 2",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "green/wiese2_00.png" },
   dither_layer = 40,
}

world:new_terrain_type{
   name = "wiese3",
   descname = _ "Meadow 3",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "green/wiese3_00.png" },
   dither_layer = 40,
}

world:new_terrain_type{
   name = "wiese4",
   descname = _ "Meadow 4",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "green/wiese4_00.png" },
   dither_layer = 40,
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
   dither_layer = 20,
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
   dither_layer = 20,
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
   dither_layer = 50,
}

world:new_terrain_type{
   name = "berg1",
   descname = _ "Mountain 1",
   editor_category = "green",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/berg1_00.png" },
   dither_layer = 60,
}

world:new_terrain_type{
   name = "berg2",
   descname = _ "Mountain 2",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/berg2_00.png" },
   dither_layer = 60,
}

world:new_terrain_type{
   name = "berg3",
   descname = _ "Mountain 3",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/berg3_00.png" },
   dither_layer = 40,
}

world:new_terrain_type{
   name = "berg4",
   descname = _ "Mountain 4",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "green/berg4_00.png" },
   dither_layer = 60,
}

world:new_terrain_type{
   name = "sumpf",
   descname = _ "Swamp",
   editor_category = "green",
   is = "unpassable",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 20,
   textures = path.glob(pics_dir .. "green", "sumpf_\\d+.png"),
   dither_layer = 30,
   fps = 14,
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
   dither_layer = 10,
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
   dither_layer = 70,
}

world:new_terrain_type{
   name = "lava",
   descname = _ "Lava",
   editor_category = "green",
   is = "acid",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.glob(pics_dir .. "green", "lava_\\d+.png"),
   dither_layer = 80,
   fps = 4,
}

world:new_terrain_type{
   name = "wasser",
   descname = _ "Water",
   editor_category = "green",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.glob(pics_dir .. "green", "wasser_\\d+.png"),
   dither_layer = 0,
   fps = 14,
}
------------------------
--  Former blackland  --
------------------------

world:new_terrain_type{
   name = "ashes",
   descname = _"Ashes",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 5,
   textures = { pics_dir .. "wasteland/ashes_00.png" },
   dither_layer = 20,
}

world:new_terrain_type{
   name = "ashes2",
   descname = _"Ashes 2",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 4,
   textures = { pics_dir .. "wasteland/ashes2_00.png" },
   dither_layer = 20,
}

world:new_terrain_type{
   name = "hardground1",
   descname = _ "Hard Ground 1",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground1_00.png" },
   dither_layer = 30,
}

world:new_terrain_type{
   name = "hardground2",
   descname = _ "Hard Ground 2",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground2_00.png" },
   dither_layer = 30,
}

world:new_terrain_type{
   name = "hardground3",
   descname = _ "Hard Ground 3",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground3_00.png" },
   dither_layer = 30,
}

world:new_terrain_type{
   name = "hardground4",
   descname = _ "Hard Ground 4",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground4_00.png" },
   dither_layer = 30,
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
   dither_layer = 40,
}

world:new_terrain_type{
   name = "wasteland_mountain1",
   descname = _ "Mountain 1",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain1_00.png" },
   dither_layer = 50,
}

world:new_terrain_type{
   name = "wasteland_mountain2",
   descname = _ "Mountain 2",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain2_00.png" },
   dither_layer = 50,
}

world:new_terrain_type{
   name = "wasteland_mountain3",
   descname = _ "Mountain 3",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain3_00.png" },
   dither_layer = 50,
}

world:new_terrain_type{
   name = "wasteland_mountain4",
   descname = _ "Mountain 4",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain4_00.png" },
   dither_layer = 50,
}

world:new_terrain_type{
   name = "wasteland_beach",
   descname = _ "Beach (Wasteland)",
   editor_category = "wasteland",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/strand_00.png" },
   dither_layer = 0,
}

world:new_terrain_type{
   name = "lava-stone1",
   descname = _ "Lava Rocks 1",
   editor_category = "wasteland",
   is = "acid",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/lava-stone1_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "lava-stone2",
   descname = _ "Lava Rocks 2",
   editor_category = "wasteland",
   is = "acid",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/lava-stone2_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "wasteland_water",
   descname = _ "Water (Wasteland)",
   editor_category = "wasteland",
   is = "water",
   valid_resources = { "fish" },
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.glob(pics_dir .. "wasteland", "water_\\d+.png"),
   dither_layer = 0,
   fps = 14,
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
   dither_layer = 50,
}

world:new_terrain_type{
   name = "tundra2",
   descname = _ "Tundra 2",
   editor_category = "winter",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra2_00.png" },
   dither_layer = 50,
}

world:new_terrain_type{
   name = "tundra3",
   descname = _ "Tundra 3",
   editor_category = "winter",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra3_00.png" },
   dither_layer = 50,
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
   dither_layer = 50,
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
   dither_layer = 50,
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
   dither_layer = 60,
}

world:new_terrain_type{
   name = "winter_mountain1",
   descname = _ "Mountain 1 (Winter)",
   editor_category = "winter",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain1_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "winter_mountain2",
   descname = _ "Mountain 2 (Winter)",
   editor_category = "winter",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain2_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "winter_mountain3",
   descname = _ "Mountain 3 (Winter)",
   editor_category = "winter",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain3_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "winter_mountain4",
   descname = _ "Mountain 4 (Winter)",
   editor_category = "winter",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain4_00.png" },
   dither_layer = 70,
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
   dither_layer = 30,
}

world:new_terrain_type{
   name = "winter_beach",
   descname = _ "Beach (Winter)",
   editor_category = "winter",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/strand_00.png" },
   dither_layer = 40,
}

world:new_terrain_type{
   name = "ice_floes",
   descname = _ "Ice Floes",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.glob(pics_dir .. "winter", "ice_flows_\\d+.png"),
   dither_layer = 10,
   fps = 5,
}

world:new_terrain_type{
   name = "ice_floes2",
   descname = _ "Ice Floes 2",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.glob(pics_dir .. "winter", "ice_flows2_\\d+.png"),
   dither_layer = 20,
   fps = 5,
}

world:new_terrain_type{
   name = "winter_water",
   descname = _ "Water (Winter)",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.glob(pics_dir .. "winter", "water_\\d+.png"),
   dither_layer = 0,
   fps = 8,
}

---------------------
--  Former Desert  --
---------------------

world:new_terrain_type{
   name = "desert4",
   descname = _ "Desert 4",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 2,
   textures = { pics_dir .. "desert/desert4_00.png" },
   dither_layer = 20,
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
   dither_layer = 30,
}

world:new_terrain_type{
   name = "desert_steppe",
   descname = _ "Steppe (Desert)",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 5,
   textures = { pics_dir .. "desert/steppe_00.png" },
   dither_layer = 30,
}

world:new_terrain_type{
   name = "meadow",
   descname = _ "Meadow (Desert)",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "desert/meadow_00.png" },
   dither_layer = 40,
}

world:new_terrain_type{
   name = "mountainmeadow",
   descname = _ "Mountain Meadow (Desert)",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountainmeadow_00.png" },
   dither_layer = 50,
}

world:new_terrain_type{
   name = "highmountainmeadow",
   descname = _ "High Mountain Meadow (Desert)",
   editor_category = "desert",
   is = "green",
   valid_resources = {"water"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/highmountainmeadow_00.png" },
   dither_layer = 60,
}

world:new_terrain_type{
   name = "mountain1",
   descname = _ "Mountain 1 (Desert)",
   editor_category = "desert",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain1_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "mountain2",
   descname = _ "Mountain 2 (Desert)",
   editor_category = "desert",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain2_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "mountain3",
   descname = _ "Mountain 3 (Desert)",
   editor_category = "desert",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain3_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "mountain4",
   descname = _ "Mountain 4 (Desert)",
   editor_category = "desert",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain4_00.png" },
   dither_layer = 70,
}

world:new_terrain_type{
   name = "desert1",
   descname = _ "Desert 1",
   editor_category = "desert",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert1_00.png" },
   dither_layer = 20,
}

world:new_terrain_type{
   name = "desert2",
   descname = _ "Desert 2",
   editor_category = "desert",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert2_00.png" },
   dither_layer = 20,
}

world:new_terrain_type{
   name = "desert3",
   descname = _ "Desert 3",
   editor_category = "desert",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert3_00.png" },
   dither_layer = 20,
}

world:new_terrain_type{
   name = "desert_beach",
   descname = _ "Beach (Desert)",
   editor_category = "desert",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/beach_00.png" },
   dither_layer = 10,
}

world:new_terrain_type{
   name = "desert_water",
   descname = _ "Water (Desert)",
   editor_category = "desert",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.glob(pics_dir .. "desert", "wasser_\\d+.png"),
   dither_layer = 0,
   fps = 5,
}
