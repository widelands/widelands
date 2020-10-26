push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "winter_beach",
   descname = _ "Beach",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 40,
   temperature = 60,
   humidity = 500,
   fertility = 100,
}

pop_textdomain()
