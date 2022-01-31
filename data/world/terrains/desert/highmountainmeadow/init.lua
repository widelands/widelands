push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "highmountainmeadow",
   descname = _("High Mountain Meadow"),
   is = "arable",
   valid_resources = {"resource_water"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 151,
   temperature = 140,
   humidity = 400,
   fertility = 400,

   enhancement = { amazons = "mountainmeadow" }
}

pop_textdomain()
