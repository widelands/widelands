-- NOCOM fill with content
include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

return {
   func = function(tree_name)
      set_textdomain("widelands")
      local world = wl.World();
      local tree = wl.Editor():get_immovable_description(tree_name)
      local result = ""
      if (tree.has_terrain_affinity) then
			result = result .. p("Pickiness:" .. " " .. tree.pickiness)
			result = result .. p("Preferred fertility:" .. " " .. tree.preferred_fertility)
			result = result .. p("Preferred humidity:" .. " " .. tree.preferred_humidity)
			result = result .. p("Preferred temperature:" .. " " .. tree.preferred_temperature)
      end
      return rt(result)
   end
}
