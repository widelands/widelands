world:new_terrain_type{
   name = "mountain3",
   descname = _ "Mountain 3",
   editor_category = "desert",
   is = "mineable",
   valid_resources = { "resource_coal", "resource_iron", "resource_gold", "resource_stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 130,
   temperature = 130,
   humidity = 50,
   fertility = 50,

   enhancement = "desert_forested_mountain2"
}
