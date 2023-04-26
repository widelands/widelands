push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "lava-stone2",
   descname = _("Lava Rocks 2"),
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(path.dirname(__file__) .. "lava-stone2_??.png"),
   dither_layer = 10,
   fps = 7,
   temperature = 1273,
   humidity = 1,
   fertility = 1,
}

pop_textdomain()
