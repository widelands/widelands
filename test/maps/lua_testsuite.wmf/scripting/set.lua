-- =======================================================================
--                                Set test
-- =======================================================================
include "scripting/set.lua"

set_basic_tests = lunit.TestCase("Set Basics")
function set_basic_tests:test_creation()
   local s = Set:new{
      map:get_field(10,12), map:get_field(11, 12),
      map:get_field(11,12), map:get_field(13, 12),
      map:get_field(14, 12)
   }

   assert_equal(4, s.size)
end

set_tests = lunit.TestCase("Set Tests")
function set_tests:setup()
   local f1 = map:get_field(10,12); self.f1 = f1
   local f2 = map:get_field(11,12); self.f2 = f2
   local f3 = map:get_field(13,12); self.f3 = f3
   local f4 = map:get_field(13,15); self.f4 = f4
   local f5 = map:get_field(15,16); self.f5 = f5
   local f6 = map:get_field(1 ,16); self.f6 = f6

   self.s1 = Set:new{ f1, f2, f2, f3, f4 }
   self.s2 = Set:new{ f4, f3, f4, f4, f5, f6 }
   self.s3 = Set:new{ f1, f2, f2, f3, f4 }
   self.empty = Set:new()
end
function set_tests:test_size()
   assert_equal(0, self.empty.size)
   assert_equal(4, self.s1.size)
   assert_equal(4, self.s2.size)
   assert_equal(4, self.s3.size)
end

function set_tests:test_add()
   self.empty:add(map:get_field(10,12))
   assert_equal(1, self.empty.size)
   self.empty:add(map:get_field(10,12))
   assert_equal(1, self.empty.size)
   self.empty:add(map:get_field(11,12))
   assert_equal(2, self.empty.size)
end

function set_tests:test_discard()
   self.s1:discard(self.f1)
   assert_equal(3, self.s1.size)
   self.s1:discard(self.f2)
   assert_equal(2, self.s1.size)
   self.s1:discard(self.f2) -- Not inside, not a problem
   assert_equal(2, self.s1.size)
   self.s1:discard(self.f3)
   self.s1:discard(self.f4)
   assert_equal(0, self.s1.size)
end

function set_tests:test_contains()
   assert_equal(true, self.s1:contains(map:get_field(10,12)))
   assert_equal(false, self.s1:contains(map:get_field(20,12)))
end

function set_tests:test_equality()
   assert_equal(self.s1, self.s1)
   assert_equal(self.s1, self.s3)
   assert_equal(self.s3, self.s1)
end

function set_tests:test_inequality()
   assert_not_equal(self.s1, self.s2)
   assert_not_equal(self.s2, self.s1)
   assert_not_equal(self.s2, self.s3)
   assert_not_equal(self.s3, self.s2)
end

function set_tests:test_union()
   local f1, f2, f3, f4, f5, f6 = self.f1, self.f2, self.f3,
      self.f4, self.f5, self.f6
   assert_equal(Set:new{f1,f2,f3,f4,f5,f6}, self.s1 + self.s2)
   assert_equal(Set:new{f1,f2,f3,f4,f5,f6}, self.s2 + self.s3)
   assert_equal(self.s1, self.s1 + self.s3)
end

function set_tests:test_difference()
   assert_equal(Set:new{self.f1, self.f2}, self.s1 - self.s2)
   assert_equal(Set:new{self.f5, self.f6}, self.s2 - self.s1)
   assert_equal(Set:new{}, self.s1 - self.s3)
   assert_equal(self.s3, self.s1 - Set:new{} - self.empty)
end

function set_tests:test_intersection()
   assert_equal(Set:new{self.f3, self.f4}, self.s1 * self.s2)
   assert_equal(Set:new{self.f3, self.f4}, self.s2 * self.s1)
   assert_equal(self.s1, self.s1 * self.s3)
   assert_equal(self.empty, self.s1 * self.empty)
end

function set_tests:test_iteration()
   local cnt = 0
   for f in self.s1:items() do
      cnt = cnt + 1
   end
   assert_equal(4, cnt)
end

function set_tests:test_pop_at()
   self.s1:pop_at(4)
   self.s1:pop_at(2)
   self.s1:pop_at(2)
   self.s1:pop_at(1)
   assert_equal(0, self.s1.size)
end
