dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "artifact02",
   descname = _ "Artifact 3",
   editor_category = "artifacts",
   size = "small",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 14, 20 },
      },
   }
}
