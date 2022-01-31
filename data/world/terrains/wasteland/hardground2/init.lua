push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "hardground2",
   descname = _("Hard Ground 2"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 370,
   temperature = 95,
   humidity = 150,
   fertility = 850,
}

pop_textdomain()
