world:new_terrain_type{
   name = "summer_mountain_meadow",
   descname = _ "Mountain Meadow",
   editor_category = "summer",
   is = "arable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 160,
   temperature = 75,
   humidity = 800,
   fertility = 450,

   enhancement = "summer_meadow1"
}
