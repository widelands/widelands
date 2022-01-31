push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "ashes2",
   descname = _("Ashes 2"),
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 4,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 410,
   temperature = 118,
   humidity = 130,
   fertility = 999,

   enhancement = { amazons = "hardground1" }
}

pop_textdomain()
