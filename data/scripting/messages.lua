-- RST
-- messages.lua
-- --------------
--
-- Functions to send messages to the player and to add objectives to campaigns.

include "scripting/coroutine.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

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
--                     The popup parameter must be set.
--
function send_message(player, title, body, parameters)
   if (parameters["popup"]) then
      wait_for_roadbuilding()
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
   wait_for_roadbuilding()
   -- In case the user input was forbidden for some reason, allow him to close the message box.
   -- While the message box is shown, the user cannot do anything else anyway.
   local user_input = wl.ui.get_user_input_allowed()
   wl.ui.set_user_input_allowed(true)
   player:message_box(title, body, parameters)
   wl.ui.set_user_input_allowed(user_input)
end

-- RST
-- .. function:: campaign_message_box(message)
--
--    Sets message.h and message.w if not set and calls
--    message_box(player, title, body, parameters) for player 1
--
--    :arg message: the message to be sent
--
function campaign_message_box(message)
   if not message.h then message.h = 400 end
   if not message.w then message.w = 450 end
   message_box(wl.Game().players[1], message.title, message.body, message)
end


-- RST
-- .. function:: add_campaign_objective(objective)
--
--    Adds an objective to a campaign.
--
--    :arg objective: The objective to be added. If the variable obj_name exists, obj_name, obj_title and obj_body are used. Otherwise, it needs to have a name, title, and body.
--
--    :returns: The new objective.
--
function add_campaign_objective(objective)
   if objective.obj_name then
      return wl.Game().players[1]:add_objective(objective.obj_name, objective.obj_title, objective.obj_body)
   else
      return wl.Game().players[1]:add_objective(objective.name, objective.title, objective.body)
   end
end

-- RST
-- .. function:: set_objective_done(objective[, sleeptime])
--
--    Sets an objectve as done and sleeps for a bit.
--
--    :arg objective: The objective to be marked as done.
--    :arg sleeptime: The milliseconds to sleep. Defaults to 3000.
--
function set_objective_done(objective, sleeptime)
   if not sleeptime then
      sleep(3000)
   else
      sleep(sleeptime)
   end
   objective.done = true
end


-- RST
-- .. function:: message_box_objective(player, message)
--
--    Calls message_box(player, message.title, message.body, message). Also adds an objective defined in obj_name, obj_title and obj_body.
--    This method should gather all options that are used often to avoid reimplementation in every single scenario script.
--
--    :arg player: the recipient of the message
--    :arg message: a table that contains all information
--
--    Besides the normal message arguments (see wl.Game.Player:message_box) the following ones can be used:
--
--    :arg position: A string that indicates at which border of the screen the message box shall appear. Can be "top", "bottom", "right", "left" or a combination (e.g. "topright"). Overrides posx and posy. Default: Center. If only one direction is indicated, the other one stays central.
--    :arg scroll_back: If true, the view scrolls/jumps back to where it came from. If false, the new location stays on the screen when the message box is closed. Default: False.
--    :arg show_instantly: If true, the message box is shown immediately. If false, this function calls message_box(), which waits until the player leaves the roadbuilding mode. Use this with care because it can be very interruptive. Default: false.
--
--    :returns: the objective if defined, nil otherwise

-- TODO(wl-zocker): This function should be used by all tutorials, campaigns and scenario maps
function message_box_objective(player, message)
   message.show_instantly = message.show_instantly or false
   message.scroll_back = message.scroll_back or false
   message.append_objective = message.append_objective or false
   message.h = message.h or 400
   message.w = message.w or 450

   local center_pixel

   if message.field then
      -- This is necessary. Otherwise, we would scroll and then wait until the road is finished.
      -- In this time, could user can scroll elsewhere, giving weird results.
      if not message.show_instantly then
         wait_for_roadbuilding()
      end
      center_pixel = scroll_to_field(message.field);
   end

   if message.position then
      if string.find(message.position,"top") then
         message.posy = 0
      elseif string.find(message.position,"bottom") then
         message.posy = 10000
      end
      if string.find(message.position,"left") then
         message.posx = 0
      elseif string.find(message.position,"right") then
         message.posx = 10000
      end
   end

   if message.show_instantly then
      -- message_box takes care of this, but player:message_box does not
      local user_input = wl.ui.get_user_input_allowed()
      wl.ui.set_user_input_allowed(true)
      player:message_box(message.title, message.body, message)
      wl.ui.set_user_input_allowed(user_input)
   else
      message_box(plr, message.title, message.body, message)
   end

   if (message.field and message.scroll_back) then
      scroll_to_map_pixel(center_pixel);
   end

   if message.obj_name then
      return add_campaign_objective(message)
   else
      return nil
   end
end
