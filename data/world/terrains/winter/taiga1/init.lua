push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "winter_taiga1",
   descname = _("Taiga 1"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 231,
   temperature = 50,
   humidity = 850,
   fertility = 450,
}

pop_textdomain()
