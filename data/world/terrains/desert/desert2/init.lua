push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "desert2",
   descname = _("Desert 2"),
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 280,
   temperature = 168,
   humidity = 1,
   fertility = 1,
}

pop_textdomain()
