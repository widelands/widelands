wl.World():new_terrain_type{
   name = "hardlava",
   descname = _ "Igneous Rocks",
   is = "arable",
   valid_resources = { "resource_water" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 360,
   temperature = 120,
   humidity = 100,
   fertility = 200,

   enhancement = "drysoil"
}
