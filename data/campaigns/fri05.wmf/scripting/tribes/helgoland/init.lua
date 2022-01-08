push_textdomain("scenario_fri05.wmf")

terraindir = "campaigns/fri05.wmf/" .. "scripting/tribes/helgoland/"--path.dirname(__file__)

wl.Descriptions():new_terrain_type {
   name = "helgoland",
   descname = "Helgoland",  -- no need to make this translatable
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { terraindir .. "helgoland.png" },
   dither_layer = 500,
   temperature = 900,
   humidity = 50,
   fertility = 200,
}

pop_textdomain()
