wl.World():new_terrain_type{
   name = "taiga",
   descname = _ "Taiga",
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 250,
   temperature = 35,
   humidity = 750,
   fertility = 300,

   enhancement = "tundra_taiga"
}
