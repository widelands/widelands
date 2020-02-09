include "scripting/richtext.lua"

function get_editor_tips()
   include "txts/tips/editor.lua"
   return tips
end

local text = ""
for index, contents in pairs(get_editor_tips()) do
   text = text .. li(contents["text"])
end

set_textdomain("widelands_editor")
return {
  title = _"Tips",
  text = text
}
