push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "ice_floes",
   descname = _("Ice Floes 1"),
   is = "water",
   valid_resources = {"resource_fish"},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(path.dirname(__file__) .. "ice_floes_??.png"),
   dither_layer = 210,
   fps = 5,
   temperature = 50,
   humidity = 999,
   fertility = 1,
   enhancement = { diking = "ice" }
}

pop_textdomain()
