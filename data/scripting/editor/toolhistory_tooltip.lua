-- RST
-- toolhistory_tooltip.lua
-- -----------------------
--
-- This script returns a compact formatted entry for toolhistory list tooltips in the editor.
-- Pass map object type ("terrain", "critter", or "immovable") followed by the internal names of
-- the map objects to the coroutine.

include "scripting/richtext.lua"
include "scripting/help.lua"

return {
   func = function(mo_type, ...)
      push_textdomain("widelands_editor")
      local result = ""
      for i,name in ipairs{...} do
         if mo_type == "terrain" then
            local descr = wl.Editor():get_terrain_description(name)
            result = result .. img(descr.representative_image) ..space(2)

         elseif mo_type == "critter" then
            result = result .. img_object(name) ..space(2)

         elseif mo_type == "immovable" then
            result = result .. img_object(name) ..space(2)

         end

      end


      -- TRANSLATORS: Help tooltip in tool history window.
      tooltip = vspace(3) .. _("Click to restore, Shfit + Click to delete, and Ctrl + Click to pin an item.")

      if result ~= "" then
         -- TRANSLATORS: Help tooltip header in tool history window.
         tooltip = tooltip .. h3(_("Selection")) .. vspace(3) .. p(result)
      end

      pop_textdomain()

      return {
         text = tooltip
      }
   end
}
