push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "hardground3",
   descname = _("Hard Ground 3"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 380,
   temperature = 105,
   humidity = 200,
   fertility = 900,
}

pop_textdomain()
