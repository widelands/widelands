wl.World():new_terrain_type{
   name = "tundra3",
   descname = _ "Tundra 3",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "resource_water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 240,
   temperature = 50,
   humidity = 800,
   fertility = 400,
}
