dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "track_winter",
   descname = _ "Track",
   -- category = "miscellaneous",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 30, 15 },
      },
   }
}
