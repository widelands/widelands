world:new_terrain_type{
   name = "summer_snow",
   descname = _ "Snow",
   editor_category = "summer",
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. pics_dir .. "idle.png" },
   dither_layer = 220,
   temperature = 50,
   humidity = 999,
   fertility = 1,
}
