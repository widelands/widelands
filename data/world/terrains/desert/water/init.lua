push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "desert_water",
   descname = _("Water"),
   is = "water",
   valid_resources = {"resource_fish"},
   default_resource = "resource_fish",
   default_resource_amount = 4,
   textures = path.list_files(path.dirname(__file__) .. "water_??.png"),
   dither_layer = 200,
   fps = 5,
   temperature = 150,
   humidity = 999,
   fertility = 1,
}

pop_textdomain()
