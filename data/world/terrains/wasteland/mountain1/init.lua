push_textdomain("world")

wl.World():new_terrain_type{
   name = "wasteland_mountain1",
   descname = _ "Mountain 1",
   is = "mineable",
   valid_resources = { "resource_coal", "resource_iron", "resource_gold", "resource_stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 92,
   temperature = 80,
   humidity = 50,
   fertility = 200,

   enhancement = "wasteland_forested_mountain1"
}

pop_textdomain()
