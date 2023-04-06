push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_steppe",
   descname = _("Steppe"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 5,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 330,
   temperature = 100,
   humidity = 400,
   fertility = 400,

   enhancement = { amazons = "summer_mountain_meadow" }
}

pop_textdomain()
