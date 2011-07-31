-- RST
-- metaserver.lua
-- -------------
--
-- This script contains some functions to communicate with the meta server


-- RST
-- .. function:: make_extra_data(plr, name, version[, extra])
--
--    Constructs a string to be send to the meta server as extra data.
--
--    :arg plr: Player to calculate extra data for
--    :type plr: :class:`~wl.game.Player`
--    :arg name: Name of the win-condition
--    :type name: :class:`string`
--    :arg version: Version the win-condition
--    :type version: :class:`integer`
--    :arg extra: list of other extra arguments that should be passed
--       to the server. They will also be incorporated into the extra string.
--    :type extra: :class:`array`
--
--    :returns: the extra string that can be passed on
function make_extra_data(plr, name, version, extra)
   extra = extra or {}
   local rv = {
      "win_condition=" .. name,
      "win_condition_version=" .. version,
      "tribe=" .. plr.tribe,
   }

   for k,v in pairs(extra) do
      rv[#rv+1] = ("%s=%s"):format(k,v)
   end

   return table.concat(rv, ";")
end

