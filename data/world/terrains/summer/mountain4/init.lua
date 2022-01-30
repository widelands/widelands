push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_mountain4",
   descname = _("Mountain 4"),
   is = "mineable",
   valid_resources = {"resource_coal", "resource_iron", "resource_gold", "resource_stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 75,
   temperature = 80,
   humidity = 100,
   fertility = 100,

   enhancement = { amazons = "summer_forested_mountain2" }
}

pop_textdomain()
