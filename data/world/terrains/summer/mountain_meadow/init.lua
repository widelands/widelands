push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_mountain_meadow",
   descname = _("Mountain Meadow"),
   is = "arable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 160,
   temperature = 75,
   humidity = 800,
   fertility = 450,

   enhancement = { amazons = "summer_meadow1" }
}

pop_textdomain()
