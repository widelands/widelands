push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "summer_swamp",
   descname = _("Swamp"),
   is = "unwalkable",
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 20,
   textures = path.list_files(path.dirname(__file__) .. "swamp_??.png"),
   dither_layer = 371,
   fps = 14,
   temperature = 105,
   humidity = 999,
   fertility = 100,
}

pop_textdomain()
