-- RST
-- messages.lua
-- --------------
--
-- Functions to send messages to the player.

-- RST
-- .. function:: send_message(player, title, body, parameters)
--
--    Sends a message to the player.
--    If the popup parameter is true and the player is in building mode,
--    the function waits until the player leaves the building mode
--    before sending the message
--
--    :arg player: the recipient of the message
--    :arg title: the localized title of the message
--    :type title: :class:`string`
--    :arg body: the localized body of the message. You can use rt functions here.
--    :type body: :class:`string`
--    :arg parameters: Array of message parameters as defined in the Lua interface,
--                     for wl.game.Player, e.g. { field = f, popup = true }.
--
function send_message(player, title, body, parameters)
   if (parameters["popup"]) then
      while (wl.ui.MapView().is_building_road) do sleep(2000) end
   end
   player:send_message(title, body, parameters)
end


-- RST
-- .. function:: message_box(player, title, message, parameters)
--
--    Waits if player is in building mode, then shows a scenario message box
--
--    :arg player: the recipient of the message
--    :arg title: the localized title of the message
--    :type title: :class:`string`
--    :arg body: the localized body of the message. You can use rt functions here.
--    :type body: :class:`string`
--    :arg parameters: Array of message parameters as defined in the Lua interface,
--                     for wl.game.Player, e.g. { field = f }.
--
function message_box(player, title, body, parameters)
   while (wl.ui.MapView().is_building_road) do sleep(2000) end
   player:message_box(title, body, parameters)
end
