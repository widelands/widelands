dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "artifact00",
   descname = _ "Artifact 1",
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
