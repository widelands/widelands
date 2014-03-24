------------------------
--  Former greenland  --
------------------------

-- NOCOM(#sirver): this is suboptimal
-- Order does matter here, since this file should stay compatible
-- with S2
--

-- NOCOM(#sirver): no need to keep the german names now - translate to english.

world:new_terrain_type{
   name = "wiese1",
   descname = _ "Meadow 1",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { "world/greenland_pics/wiese1_00.png" },
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "wiese2",
   descname = _ "Meadow 2",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { "world/greenland_pics/wiese2_00.png" },
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "wiese3",
   descname = _ "Meadow 3",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { "world/greenland_pics/wiese3_00.png" },
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "wiese4",
   descname = _ "Meadow 4",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { "world/greenland_pics/wiese4_00.png" },
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "steppe",
   descname = _ "Steppe",
   editor_category = "green",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 5,
   textures = { "world/greenland_pics/steppe_00.png" },
   dither_layer = 20,
   fps = 0,
}

world:new_terrain_type{
   name = "steppe_kahl",
   descname = _ "Barren Steppe",
   editor_category = "green",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 4,
   textures = { "world/greenland_pics/steppe_kahl_00.png" },
   dither_layer = 20,
   fps = 0,
}

world:new_terrain_type{
   name = "bergwiese",
   descname = _ "Mountain Meadow",
   editor_category = "green",
   is = "green",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/greenland_pics/bergwiese_00.png" },
   dither_layer = 50,
   fps = 0,
}

world:new_terrain_type{
   name = "berg1",
   descname = _ "Mountain 1",
   editor_category = "green",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/greenland_pics/berg1_00.png" },
   dither_layer = 60,
   fps = 0,
}

world:new_terrain_type{
   name = "berg2",
   descname = _ "Mountain 2",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/greenland_pics/berg2_00.png" },
   dither_layer = 60,
   fps = 0,
}

world:new_terrain_type{
   name = "berg3",
   descname = _ "Mountain 3",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/greenland_pics/berg3_00.png" },
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "berg4",
   descname = _ "Mountain 4",
   editor_category = "green",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/greenland_pics/berg4_00.png" },
   dither_layer = 60,
   fps = 0,
}

world:new_terrain_type{
   name = "sumpf",
   descname = _ "Swamp",
   editor_category = "green",
   is = "unpassable",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 20,
   textures = path.glob("world/greenland_pics", "sumpf_*.png"),
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
   textures = { "world/greenland_pics/strand_00.png" },
   dither_layer = 10,
   fps = 0,
}

world:new_terrain_type{
   name = "schnee",
   descname = _ "Snow",
   editor_category = "green",
   is = "dead",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/greenland_pics/schnee_00.png" },
   dither_layer = 70,
   fps = 0,
}

world:new_terrain_type{
   name = "lava",
   descname = _ "Lava",
   editor_category = "green",
   is = "acid",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.glob("world/greenland_pics", "lava_*.png"),
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
   textures = path.glob("world/greenland_pics", "wasser_*.png"),
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
   textures = { "world/blackland_pics/ashes_00.png" },
   dither_layer = 20,
   fps = 0,
}

world:new_terrain_type{
   name = "ashes2",
   descname = _"Ashes 2",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 4,
   textures = { "world/blackland_pics/ashes2_00.png" },
   dither_layer = 20,
   fps = 0,
}

world:new_terrain_type{
   name = "hardground1",
   descname = _ "Hard Ground 1",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { "world/blackland_pics/hardground1_00.png" },
   dither_layer = 30,
   fps = 0,
}

world:new_terrain_type{
   name = "hardground2",
   descname = _ "Hard Ground 2",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { "world/blackland_pics/hardground2_00.png" },
   dither_layer = 30,
   fps = 0,
}

world:new_terrain_type{
   name = "hardground3",
   descname = _ "Hard Ground 3",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { "world/blackland_pics/hardground3_00.png" },
   dither_layer = 30,
   fps = 0,
}

world:new_terrain_type{
   name = "hardground4",
   descname = _ "Hard Ground 4",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { "world/blackland_pics/hardground4_00.png" },
   dither_layer = 30,
   fps = 0,
}

world:new_terrain_type{
   name = "hardlava",
   descname = _ "Igneous Rocks",
   editor_category = "wasteland",
   is = "green",
   valid_resources = { "water" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/blackland_pics/hardlava_00.png" },
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "wasteland_mountain1",
   descname = _ "Mountain 1",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/blackland_pics/mountain1_00.png" },
   dither_layer = 50,
   fps = 0,
}

world:new_terrain_type{
   name = "wasteland_mountain2",
   descname = _ "Mountain 2",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/blackland_pics/mountain2_00.png" },
   dither_layer = 50,
   fps = 0,
}

world:new_terrain_type{
   name = "wasteland_mountain3",
   descname = _ "Mountain 3",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/blackland_pics/mountain3_00.png" },
   dither_layer = 50,
   fps = 0,
}

world:new_terrain_type{
   name = "wasteland_mountain4",
   descname = _ "Mountain 4",
   editor_category = "wasteland",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/blackland_pics/mountain4_00.png" },
   dither_layer = 50,
   fps = 0,
}

world:new_terrain_type{
   name = "wasteland_beach",
   descname = _ "Beach (Wasteland)",
   editor_category = "wasteland",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/blackland_pics/strand_00.png" },
   dither_layer = 0,
   fps = 0,
}

world:new_terrain_type{
   name = "lava-stone1",
   descname = _ "Lava Rocks 1",
   editor_category = "wasteland",
   is = "acid",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/blackland_pics/lava-stone1_00.png" },
   dither_layer = 70,
   fps = 0,
}

world:new_terrain_type{
   name = "lava-stone2",
   descname = _ "Lava Rocks 2",
   editor_category = "wasteland",
   is = "acid",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { "world/blackland_pics/lava-stone2_00.png" },
   dither_layer = 70,
   fps = 0,
}

world:new_terrain_type{
   name = "wasteland_water",
   descname = _ "Water (Wasteland)",
   editor_category = "wasteland",
   is = "water",
   valid_resources = { "fish" },
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.glob("world/blackland_pics", "water_*.png"),
   dither_layer = 0,
   fps = 14,
}

-------------------------
--  Former Winterland  --
-------------------------


---------------------
--  Former Desert  --
---------------------
