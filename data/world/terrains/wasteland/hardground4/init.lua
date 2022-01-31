push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "hardground4",
   descname = _("Hard Ground 4"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 390,
   temperature = 90,
   humidity = 200,
   fertility = 800,
}

pop_textdomain()
