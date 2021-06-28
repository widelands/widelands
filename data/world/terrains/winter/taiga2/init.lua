push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "winter_taiga2",
   descname = _("Taiga 2"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 240,
   temperature = 55,
   humidity = 750,
   fertility = 450,
}

pop_textdomain()
