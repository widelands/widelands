-- =====================================================
-- Helper functions that are only used in this tutorial
-- =====================================================

-- These are very special functions that simplify demonstrations to the user.
-- This includes the UserInputDisabler, some functions for mouse movement and
-- the Sentry Thread, which forbids the player to build stuff he shouldn't.


-- A small helper class to disable/enable autosaving and user interaction
UserInputDisabler = {}
function UserInputDisabler:new()
   local rv = {}
   setmetatable(rv, self)
   self.__index = self

   rv:establish_blocks()

   return rv
end
function UserInputDisabler:establish_blocks()
   self._ui_state = wl.ui.get_user_input_allowed()
   self._as_state = wl.Game().allow_saving
   self._game_speed = wl.Game().desired_speed

   wl.ui.set_user_input_allowed(false)
   wl.Game().allow_saving = false
   -- whatever we do, we want it to happen slowly
   wl.Game().desired_speed = 1000
end
function UserInputDisabler:lift_blocks()
   wl.ui.set_user_input_allowed(self._ui_state)
   wl.Game().allow_saving = self._as_state
   wl.Game().desired_speed = self._game_speed
end

function click_on_field(f)
   local sleeptime = 500

   local blocker = UserInputDisabler:new()

   mouse_to_field(f)
   sleep(sleeptime)

   wl.ui.MapView():click(f)
   sleep(sleeptime)

   blocker:lift_blocks()
end

function click_on_panel(panel)
   local sleeptime = 500

   local blocker = UserInputDisabler:new()

   sleep(sleeptime)
   if panel ~= nil then
      if not panel.active then -- If this is a tab and already on, do nothing
         mouse_to_panel(panel)
         sleep(sleeptime)
         if panel.press then panel:press() sleep(250) end
         if panel.click then panel:click() end
         sleep(sleeptime)
      end
   else
      print('Attempt to click on a non-existing panel.')
   end
   blocker:lift_blocks()
end


function select_item_from_dropdown(name, item)
   local blocker = UserInputDisabler:new()
   wl.ui.MapView().dropdowns[name]:select()
   sleep(3000)
   blocker:lift_blocks()
end

function open_item_from_dropdown(name, item)
   wl.ui.MapView().dropdowns[name]:open(item)
end

function show_item_from_dropdown(name, item)
   local blocker = UserInputDisabler:new()
   local ind = wl.ui.MapView().dropdowns[name].no_of_items
   wl.ui.MapView().dropdowns[name]:highlight_item(ind)
   sleep(500)
   while ind >= item do
      wl.ui.MapView().dropdowns[name]:highlight_item(ind)
      ind = ind - 1
      sleep(300)
   end
   blocker:lift_blocks()
end

-- Make sure the user is in road building mode starting from the given flag
function enter_road_building_mode(flag)
   local mv = wl.ui.MapView()

   if mv.is_building_road then
      mv:abort_road_building()
   end

   mv:start_road_building(flag)
end


-- ==============
-- Sentry Thread
-- ==============
-- This thread makes sure that the player does not build stuff where he
-- is not supposed to. He gets a message box when he tries and what he built
-- gets immediately ripped. This thread can be disabled temporarily.
function _fmt(f) return ("%i_%i"):format(f.x, f.y) end
function register_immovable_as_allowed(i)
   for idx, f in ipairs(i.fields) do
      registered_player_immovables[_fmt(f)] = true
   end

    -- buildings and constructionsite have a flag
   if is_building(i) or i.descr.type_name == "constructionsite" then
      registered_player_immovables[_fmt(i.fields[1].brn)] = true
   end
end
register_immovable_as_allowed(sf.immovable)

-- This function removes the field from the list of allowed immovables. This is
-- needed when the player destroys a flag, for example.
function unregister_field(f)
   if registered_player_immovables[_fmt(f)] then
      registered_player_immovables[_fmt(f)] = false
   end
end

function bad_boy_sentry()
   while not terminate_bad_boy_sentinel do
      -- Check all fields.
      local sent_msg = false
      for idx,f in ipairs(sf:region(8)) do
         if f.immovable and f.immovable.owner == plr then
            if not registered_player_immovables[_fmt(f)] then

               -- Give the callback a chance to veto the deletion. Maybe
               -- we expect the player to build something at the moment
               if not immovable_is_legal(f.immovable) then
                  -- scold the player
                  if not sent_msg then
                     campaign_message_box(scold_player())
                     sent_msg = true
                  end

                  -- Remove the object again
                  f.immovable:remove()

                  -- Make sure that the user is not building a road at the moment.
                  wl.ui.MapView():abort_road_building()
               end
            end
         else
            unregister_field(f)
         end
      end
      sleep(1000)
   end
end

-- Allows constructionsites for the given buildings, all others are invalid
-- as is any other immovable build by the player
function allow_constructionsite(i, buildings)
   if i.descr.type_name == "constructionsite" then
      if not buildings then return i end
      for idx,n in ipairs(buildings) do
         if i.building == n then return i end
      end
      return false
   elseif i.descr.type_name == "flag" then
      local tr = i.fields[1].tln.immovable
      if tr and tr.descr.type_name == "constructionsite" then
         return allow_constructionsite(tr, buildings)
      end
   end

   return false
end
