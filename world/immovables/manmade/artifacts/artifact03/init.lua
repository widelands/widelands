dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "artifact03",
   descname = _ "Artifact",
   editor_category = "artifacts",
   size = "small",
   attributes = { "artifact" },
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 14, 20 },
      },
   }
}
