dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "artifact00",
   descname = _ "Artifact",
   editor_category = "artifacts",
   size = "small",
   attributes = { "artifact" },
   programs = {},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 14, 20 },
      },
   }
}
