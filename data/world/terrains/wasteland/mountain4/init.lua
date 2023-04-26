push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "wasteland_mountain4",
   descname = _("Mountain 4"),
   is = "mineable",
   valid_resources = { "resource_coal", "resource_iron", "resource_gold", "resource_stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 80,
   temperature = 80,
   humidity = 50,
   fertility = 200,

   enhancement = { amazons = "wasteland_forested_mountain2" }
}

pop_textdomain()
