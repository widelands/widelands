push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_meadow2",
   descname = _("Meadow 2"),
   is = "arable",
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 350,
   temperature = 100,
   humidity = 600,
   fertility = 650,
}

pop_textdomain()
