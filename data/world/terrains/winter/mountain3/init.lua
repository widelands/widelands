push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "winter_mountain3",
   descname = _("Mountain 3"),
   is = "mineable",
   valid_resources = { "resource_coal", "resource_iron", "resource_gold", "resource_stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 100,
   temperature = 20,
   humidity = 300,
   fertility = 50,

   enhancement = { amazons = "winter_forested_mountain2" }
}

pop_textdomain()
