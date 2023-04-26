push_textdomain("world")

wl.Descriptions():new_terrain_type{
   name = "desert4",
   descname = _("Desert 4"),
   is = "arable",
   valid_resources = {"resource_water"},
   default_resource = "resource_water",
   default_resource_amount = 2,
   textures = { path.dirname(__file__) .. "idle.png" },
   dither_layer = 270,
   temperature = 168,
   humidity = 1,
   fertility = 100,

   enhancement = { amazons = "drysoil" }
}

pop_textdomain()
