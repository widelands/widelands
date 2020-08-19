world:new_terrain_type{
   name = "hardground3",
   descname = _ "Hard Ground 3",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "resource_water",
   default_resource_amount = 10,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 380,
   temperature = 105,
   humidity = 200,
   fertility = 900,
}
