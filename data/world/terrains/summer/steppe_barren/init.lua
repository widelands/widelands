push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_steppe_barren",
   descname = _("Barren Steppe"),
   is = "arable",
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 4,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 320,
   temperature = 100,
   humidity = 150,
   fertility = 150,

   enhancement = { amazons = "summer_steppe" }
}

pop_textdomain()
