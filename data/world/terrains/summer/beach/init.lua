push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_beach",
   descname = _("Beach"),
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 60,
   temperature = 120,
   humidity = 600,
   fertility = 200,
   enhancement = { diking = "summer_steppe_barren" }
}

pop_textdomain()
