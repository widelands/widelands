-- =================================
-- Player access through properties
-- =================================
-- Checks that a wl.Editor.Player is returned in Editor and vice versa
plr_access = lunit.TestCase("Player Access through properties")
function plr_access:setup()
   self.field = map:get_field(13,10)
   self.f = player1:place_flag(self.field, 1)
end
function plr_access:teardown()
   pcall(self.f.remove, self.f)
end

