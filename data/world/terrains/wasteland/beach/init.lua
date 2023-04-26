push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "wasteland_beach",
   descname = _("Beach"),
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 50,
   temperature = 60,
   humidity = 400,
   fertility = 200,
   enhancement = { diking = "hardlava" }
}

pop_textdomain()
