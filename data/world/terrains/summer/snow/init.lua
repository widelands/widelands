push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_snow",
   descname = _("Snow"),
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 220,
   temperature = 50,
   humidity = 999,
   fertility = 1,
}

pop_textdomain()
