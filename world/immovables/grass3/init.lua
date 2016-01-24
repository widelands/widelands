dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "grass3",
   descname = _ "Grass",
   editor_category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 11 },
         sound_effect = {
            directory = "sound/animals",
            name = "frog1",
         },
      },
   }
}
