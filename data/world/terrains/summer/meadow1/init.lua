push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_meadow1",
   descname = _("Meadow 1"),
   is = "arable",
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 10,

   textures = { path.dirname(__file__) .. "idle.png" },

   dither_layer = 340,

   temperature = 100,
   humidity = 600,
   fertility = 700,
}

pop_textdomain()
