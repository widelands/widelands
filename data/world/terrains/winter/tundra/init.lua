push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "winter_tundra",
   descname = _("Tundra"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 250,
   temperature = 35,
   humidity = 750,
   fertility = 300,

   enhancement = { amazons = "tundra_taiga" }
}

pop_textdomain()
