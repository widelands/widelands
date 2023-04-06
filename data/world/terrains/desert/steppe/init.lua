push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "desert_steppe",
   descname = _("Steppe"),
   is = "arable",
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 5,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 360,
   temperature = 155,
   humidity = 500,
   fertility = 500,
}

pop_textdomain()
