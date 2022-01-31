push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "lava",
   descname = _("Lava"),
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(path.dirname(__file__) .. "lava_??.png"),
   dither_layer = 30,
   fps = 4,
   temperature = 1273,
   humidity = 1,
   fertility = 1,
}

pop_textdomain()
