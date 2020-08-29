wl.World():new_terrain_type{
   name = "tundra",
   descname = _ "Tundra 1",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 230,
   temperature = 50,
   humidity = 850,
   fertility = 450,
}
