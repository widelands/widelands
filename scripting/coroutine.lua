-- =======================================================================
--          Convenience functions for Widelands Coroutine Handling          
-- =======================================================================

-- ================
-- Basic functions 
-- ================
function run(func, ...) 
   c = coroutine.create(func)
   success, sleeptime = coroutine.resume(c, ...)
   if success then
      wl.game.run_coroutine(c, sleeptime)
   else
      error(sleeptime) 
   end
end

function sleep(time)
   coroutine.yield(wl.game.get_time() + time)
end

function wake_me(at)
   coroutine.yield(at)
end

