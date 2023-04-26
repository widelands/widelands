push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "winter_forested_mountain2",
   descname = _("Forested Mountain 2"),
   is = "mineable",
   valid_resources = {"resource_coal", "resource_iron", "resource_gold", "resource_stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 106,
   temperature = 35,
   humidity = 700,
   fertility = 400,
}

pop_textdomain()
