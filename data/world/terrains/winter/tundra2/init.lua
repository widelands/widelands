wl.World():new_terrain_type{
   name = "tundra2",
   descname = _ "Tundra 2",
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
   temperature = 55,
   humidity = 750,
   fertility = 450,
}
