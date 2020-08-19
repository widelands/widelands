world:new_terrain_type{
   name = "snow",
   descname = _ "Snow",
   editor_category = "winter",
   is = "arable",
   valid_resources = { "water" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 250,
   temperature = 25,
   humidity = 800,
   fertility = 100,

   enhancement = "taiga"
}
