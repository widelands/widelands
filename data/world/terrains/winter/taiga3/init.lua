push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "winter_taiga3",
   descname = _("Taiga 3"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 241,
   temperature = 50,
   humidity = 800,
   fertility = 400,
}

pop_textdomain()
