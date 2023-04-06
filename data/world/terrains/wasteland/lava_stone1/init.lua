push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "lava-stone1",
   descname = _("Lava Rocks 1"),
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(path.dirname(__file__) .. "lava-stone1_??.png"),
   dither_layer = 20,
   fps = 7,
   temperature = 1273,
   humidity = 1,
   fertility = 1,
}

pop_textdomain()
