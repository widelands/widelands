push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "ashes1",
   descname = _("Ashes 1"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 5,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 400,
   temperature = 120,
   humidity = 150,
   fertility = 900,

   enhancement = { amazons = "hardground3" }
}

pop_textdomain()
