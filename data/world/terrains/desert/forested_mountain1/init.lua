push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "desert_forested_mountain1",
   descname = _("Forested Mountain 1"),
   is = "mineable",
   valid_resources = {"resource_coal", "resource_iron", "resource_gold", "resource_stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 69,
   temperature = 141,
   humidity = 500,
   fertility = 500,
}

pop_textdomain()
