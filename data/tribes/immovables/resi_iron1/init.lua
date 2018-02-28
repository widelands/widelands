dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "resi_iron1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Iron Vein"),
   helptext_script = dirname .. "helptexts.lua",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 7, 10 },
      },
   }
}
