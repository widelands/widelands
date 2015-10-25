dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "artifact03",
   descname = _ "Artifact 4",
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
