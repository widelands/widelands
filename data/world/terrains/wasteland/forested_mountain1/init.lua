push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "wasteland_forested_mountain1",
   descname = _("Forested Mountain 1"),
   is = "mineable",
   valid_resources = {"resource_coal", "resource_iron", "resource_gold", "resource_stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 81,
   temperature = 110,
   humidity = 150,
   fertility = 950,
}

pop_textdomain()
