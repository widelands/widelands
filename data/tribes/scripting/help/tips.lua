include "scripting/formatting.lua"

function get_general_tips()
   include "txts/tips/general_game.lua"
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

function get_singleplayer_tips()
   include "txts/tips/singleplayer.lua"
   return tips
end

function get_multiplayer_tips()
   include "txts/tips/multiplayer.lua"
   return tips
end

function format_tips(tips)
   local text = paragraphdivider()
   for index, contents in pairs(tips) do
      text = text .. listitem_bullet(contents["text"])
   end
   text = text .. "</p>"
   return text
end

return {
   func = function(tribename, game_type)
      set_textdomain("tribes_encyclopedia")
      local text = h2(_"General")
      text = text .. format_tips(get_general_tips())

      set_textdomain("tribes_encyclopedia")
      if (tribename == "atlanteans") then
         text = text .. h2(_"Atlanteans")
         text = text .. format_tips(get_atlantean_tips())
      elseif (tribename == "barbarians") then
         text = text .. h2(_"Barbarians")
         text = text .. format_tips(get_barbarian_tips())
      elseif (tribename == "empire") then
         text = text .. h2(_"Empire")
         text = text .. format_tips(get_empire_tips())
      elseif (tribename == "frisians") then
         text = text .. h2(_"Frisians")
         text = text .. format_tips(get_frisian_tips())
      end

      set_textdomain("tribes_encyclopedia")
      if (game_type == "singleplayer") then
         text = text .. h2(_"Single Player")
         text = text .. format_tips(get_singleplayer_tips())
      else
         text = text .. h2(_"Multiplayer")
         text = text .. format_tips(get_multiplayer_tips())
      end

      set_textdomain("tribes_encyclopedia")
      return {
        title = _"Tips",
        text = rt(text)
      }
   end
}
