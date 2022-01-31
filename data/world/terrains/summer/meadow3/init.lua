push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_meadow3",
   descname = _("Meadow 3"),
   is = "arable",
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 351,
   temperature = 105,
   humidity = 550,
   fertility = 800,
}

pop_textdomain()
