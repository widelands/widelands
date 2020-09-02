push_textdomain("world")

wl.World():new_terrain_type{
   name = "desert3",
   descname = _ "Desert 3",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 280,
   temperature = 178,
   humidity = 1,
   fertility = 1,
}

pop_textdomain()
