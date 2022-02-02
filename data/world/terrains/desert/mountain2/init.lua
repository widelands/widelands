push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "mountain2",
   descname = _("Mountain 2"),
   is = "mineable",
   valid_resources = { "resource_coal", "resource_iron", "resource_gold", "resource_stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 121,
   temperature = 130,
   humidity = 50,
   fertility = 50,

   enhancement = { amazons = "desert_forested_mountain1" }
}

pop_textdomain()
