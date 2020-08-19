world:new_terrain_type{
   name = "ice",
   descname = _ "Ice",
   editor_category = "winter",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 260,
   temperature = 25,
   humidity = 500,
   fertility = 100,
}
