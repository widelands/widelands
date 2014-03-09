------------------------
--  Former greenland  --
------------------------

-- NOCOM(#sirver): this is suboptimal
-- Order does matter here, since this file should stay compatible
-- with S2

world:new_terrain_type{
   name = "steppe",
   descname = _ "Steppe",
   is = "green",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 5,
   textures = "world/greenland_pics/steppe_??.png",
   dither_layer = 20,
   fps = 0,
}

world:new_terrain_type{
   name = "berg1",
   descname = _ "Mountain 1",
   is = "mountain",
   valid_resources = { "coal", "iron", "gold", "granite" },
   default_resource = "",
   default_resource_amount = 0,
   textures = "world/greenland_pics/berg1_??.png",
   dither_layer = 60,
   fps = 0,
}

world:new_terrain_type{
   name = "schnee",
   descname = _ "Snow",
   is = "dead",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = "world/greenland_pics/schnee_??.png",
   dither_layer = 70,
   fps = 0,
}

world:new_terrain_type{
   name = "sumpf",
   descname = _ "Swamp",
   is = "unpassable",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 20,
   textures = "world/greenland_pics/sumpf_??.png",
   dither_layer = 30,
   fps = 14,
}

world:new_terrain_type{
   name = "strand",
   descname = _ "Beach",
   is = "dry",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = "world/greenland_pics/strand_??.png",
   dither_layer = 10,
   fps = 0,
}

world:new_terrain_type{
   name = "wasser",
   descname = _ "Water",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = "world/greenland_pics/wasser_??.png",
   dither_layer = 0,
   fps = 14,
}

world:new_terrain_type{
   name = "wiese1",
   descname = _ "Meadow 1",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = "world/greenland_pics/wiese1_??.png",
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "wiese2",
   descname = _ "Meadow 2",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = "world/greenland_pics/wiese2_??.png",
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "wiese3",
   descname = _ "Meadow 3",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = "world/greenland_pics/wiese3_??.png",
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "berg2",
   descname = _ "Mountain 2",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = "world/greenland_pics/berg2_??.png",
   dither_layer = 60,
   fps = 0,
}

world:new_terrain_type{
   name = "berg3",
   descname = _ "Mountain 3",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = "world/greenland_pics/berg3_??.png",
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "berg4",
   descname = _ "Mountain 4",
   is = "mountain",
   valid_resources = {"coal", "iron", "gold", "granite"},
   default_resource = "",
   default_resource_amount = 0,
   textures = "world/greenland_pics/berg4_??.png",
   dither_layer = 60,
   fps = 0,
}

world:new_terrain_type{
   name = "steppe_kahl",
   descname = _ "Barren Steppe",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 4,
   textures = "world/greenland_pics/steppe_kahl_??.png",
   dither_layer = 20,
   fps = 0,
}

world:new_terrain_type{
   name = "wiese4",
   descname = _ "Meadow 4",
   is = "green",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = "world/greenland_pics/wiese4_??.png",
   dither_layer = 40,
   fps = 0,
}

world:new_terrain_type{
   name = "lava",
   descname = _ "Lava",
   is = "acid",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = "world/greenland_pics/lava_??.png",
   dither_layer = 80,
   fps = 4,
}

world:new_terrain_type{
   name = "bergwiese",
   descname = _ "Mountain Meadow",
   is = "green",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = "world/greenland_pics/bergwiese_??.png",
   dither_layer = 50,
   fps = 0,
}
