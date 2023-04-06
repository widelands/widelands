push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "desert1",
   descname = _("Desert 1"),
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 290,
   temperature = 167,
   humidity = 1,
   fertility = 1,
}

pop_textdomain()
