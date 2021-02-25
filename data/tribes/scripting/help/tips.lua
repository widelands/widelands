include "scripting/richtext.lua"

function get_general_tips()
   include "txts/tips/general_game.lua"
   return tips
end

function get_amazon_tips()
   include "txts/tips/amazons.lua"
   return tips
end

function get_atlantean_tips()
   include "txts/tips/atlanteans.lua"
   return tips
end

function get_barbarian_tips()
   include "txts/tips/barbarians.lua"
   return tips
end

function get_empire_tips()
   include "txts/tips/empire.lua"
   return tips
end

function get_frisian_tips()
   include "txts/tips/frisians.lua"
   return tips
end

function get_addon_tribe_tips(tribe)
   include "txts/tips/frisians.lua"
   return tips
end

function get_singleplayer_tips()
   include "txts/tips/singleplayer.lua"
   return tips
end

function get_multiplayer_tips()
   include "txts/tips/multiplayer.lua"
   return tips
end

function format_tips(tips)
   local text = ""
   for index, contents in pairs(tips) do
      text = text .. li(contents["text"])
   end
   return text
end

return {
   func = function(tribename, game_type)
      push_textdomain("tribes_encyclopedia")
      local text = h2(_"General")
      text = text .. format_tips(get_general_tips())

      if tribename ~= nil and tribename ~= "" then
         local descr = wl.Game():get_tribe_description(tribename)
         local scriptpath = descr.directory
         if scriptpath:find("addons") == 1 then
            include(scriptpath .. "tips.lua")
         else
            include("txts/tips/" .. tribename .. ".lua")
         end
         if tips then
            text = text .. h2(descr.descname) .. format_tips(tips)
            tips = nil
         end
      end

      if (game_type == "singleplayer") then
         text = text .. h2(_"Single Player")
         text = text .. format_tips(get_singleplayer_tips())
      else
         text = text .. h2(_"Multiplayer")
         text = text .. format_tips(get_multiplayer_tips())
      end

      local result = {
        title = _"Tips",
        text = text
      }
      pop_textdomain()
      return result
   end
}
