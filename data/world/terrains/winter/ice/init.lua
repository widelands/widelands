push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "ice",
   descname = _("Ice"),
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 260,
   temperature = 25,
   humidity = 500,
   fertility = 100,
   enhancement = { diking = "winter_beach" }
}

pop_textdomain()
