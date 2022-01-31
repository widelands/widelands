push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "drysoil",
   descname = _("Dry Soil"),
   is = "arable",
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 4,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 300,
   temperature = 172,
   humidity = 200,
   fertility = 200,

   enhancement = { amazons = "highmountainmeadow" }
}

pop_textdomain()
