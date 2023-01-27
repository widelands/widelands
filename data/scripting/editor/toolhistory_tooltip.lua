-- RST
-- toolhistory_tooltip.lua
-- -----------------------
--
-- This script returns a compact formatted entry for toolhistory list tooltips in the editor.
-- Pass map object type ("terrain", "critter", or "immovable") followed by the internal names of
-- the map objects to the coroutine.

include "scripting/richtext.lua"
include "scripting/help.lua"

local gap = styles.get_size("editor_tooltip_icon_gap")

return {
   func = function(mo_type, ...)
      push_textdomain("widelands_editor")
      local result = ""
      for i,name in ipairs{...} do
         if mo_type == "terrain" then
            local descr = wl.Editor():get_terrain_description(name)
            result = result .. img(descr.representative_image) .. space(gap)

         elseif mo_type == "critter" then
            result = result .. img_object(name) .. space(gap)

         elseif mo_type == "immovable" then
            result = result .. img_object(name) .. space(gap)

         end
      end

      if result ~= "" then
         result = p(result)
      end

      pop_textdomain()

      return {
         text = result
      }
   end
}
