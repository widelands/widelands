push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "tundra_taiga",
   descname = _("Tundra Taiga"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 230,
   temperature = 40,
   humidity = 750,
   fertility = 400,

   enhancement = { amazons = "winter_taiga2" }
}

pop_textdomain()
