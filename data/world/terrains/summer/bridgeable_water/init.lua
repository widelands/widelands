push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_bridgeable_water",
   descname = _("Bridgeable Water"),
   is = "bridgeable",
   valid_resources = {"resource_fish"},
   default_resource = "resource_fish",
   default_resource_amount = 1,
   textures = path.list_files(path.dirname(__file__) .. "bridgeable_??.png"),
   dither_layer = 181,
   fps = 10,
   temperature = 100,
   humidity = 999,
   fertility = 1,
   enhancement = { diking = "summer_beach" }
}

pop_textdomain()
