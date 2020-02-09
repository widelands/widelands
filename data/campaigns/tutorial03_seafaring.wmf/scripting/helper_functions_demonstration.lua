-- =====================================================
-- Helper functions copied from tut01
-- =====================================================

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
