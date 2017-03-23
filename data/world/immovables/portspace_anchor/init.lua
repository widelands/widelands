-- An anchor to mark a port space. These will be added automatically by
-- Widelands during map load, or when setting port spaces in the editor.
-- You can change the image, but do not remove it.

dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "portspace_anchor",
   descname = "Port Space", -- Do not localize - will not be shown in the UI
   editor_category = "immovables_none",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 35, 20 },
      },
   }
}
