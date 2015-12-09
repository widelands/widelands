-- NOCOM fill with content
include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

return {
   func = function()
      set_textdomain("widelands")
      local result = rt(h1("Not Implermented"))
      result = result .. rt(p("We want some help here"))
      return result
   end
}
