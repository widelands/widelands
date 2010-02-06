-- =======================================================================
--          Convenience functions for Widelands Coroutine Handling          
-- =======================================================================

wl.cr = {}

-- ================
-- Basic functions 
-- ================
function wl.cr.run(func, ...) 
   c = coroutine.create(func)
   success, sleeptime = coroutine.resume(c, ...)
   if success then
      wl.game.run_coroutine(c, sleeptime)
   end
end

function wl.cr.sleep(time)
   coroutine.yield(wl.game.get_time() + time)
end

function wl.cr.wake_me(at)
   coroutine.yield(at)
end

