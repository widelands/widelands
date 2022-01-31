push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "winter_water",
   descname = _("Water"),
   is = "water",
   valid_resources = {"resource_fish"},
   default_resource = "resource_fish",
   default_resource_amount = 4,
   textures = path.list_files(path.dirname(__file__) .. "water_??.png"),
   dither_layer = 190,
   fps = 8,
   temperature = 50,
   humidity = 999,
   fertility = 1,
   enhancement = { diking = "summer_beach" }
}

pop_textdomain()
