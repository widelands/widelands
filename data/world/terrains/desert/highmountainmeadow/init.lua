wl.World():new_terrain_type{
   name = "highmountainmeadow",
   descname = _ "High Mountain Meadow",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"resource_water"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 150,
   temperature = 140,
   humidity = 400,
   fertility = 400,

   enhancement = "mountainmeadow"
}
