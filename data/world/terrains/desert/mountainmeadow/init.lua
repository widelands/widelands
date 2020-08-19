world:new_terrain_type{
   name = "mountainmeadow",
   descname = _ "Mountain Meadow",
   editor_category = "desert",
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
   temperature = 145,
   humidity = 500,
   fertility = 500,

   enhancement = "desert_steppe"
}
