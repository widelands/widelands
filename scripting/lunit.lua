
--[[--------------------------------------------------------------------------

    This file is part of lunit 0.3 (alpha).

    For Details about lunit look at: http://www.nessie.de/mroth/lunit/

    Author: Michael Roth <mroth@nessie.de>

    Copyright (c) 2004 Michael Roth <mroth@nessie.de>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    This file was heavily modified for the internal use of Widelands.

--]]--------------------------------------------------------------------------

lunit = {}
lunit.testcases = {}
lunit.stats = {}

--------------------------
-- Type check functions --
--------------------------
function is_nil(x)
  return type(x) == "nil"
end

function is_boolean(x)
  return type(x) == "boolean"
end

function is_number(x)
  return type(x) == "number"
end

function is_string(x)
  return type(x) == "string"
end

function is_table(x)
  return type(x) == "table"
end

function is_function(x)
  return type(x) == "function"
end

function is_thread(x)
  return type(x) == "thread"
end

function is_userdata(x)
  return type(x) == "userdata"
end

----------------------
-- Assert functions --
----------------------
function assert_fail(msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_fail", msg)
  lunit_do_assert(false, "failure", msg)
end

function assert_true(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_true", msg)
  lunit_do_assert(is_boolean(actual), "true expected but was a "..type(actual), msg)
  lunit_do_assert(actual == true, "true expected but was false", msg)
  return actual
end

function assert_false(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_false", msg)
  lunit_do_assert(is_boolean(actual), "false expected but was a "..type(actual), msg)
  lunit_do_assert(actual == false, "false expected but was true", msg)
  return actual
end

function assert_equal(expected, actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_equal", msg)
  lunit_do_assert(expected == actual, "expected '"..tostring(expected).."' but was '"..tostring(actual).."'", msg)
  return actual
end

function assert_not_equal(unexpected, actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_equal", msg)
  lunit_do_assert(unexpected ~= actual, "'"..tostring(expected).."' not expected but was one", msg)
  return actual
end

function assert_match(pattern, actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_match", msg)
  lunit_do_assert(is_string(pattern), "assert_match expects the pattern as a string")
  lunit_do_assert(is_string(actual), "expected a string to match pattern '"..pattern.."' but was a '"..type(actual).."'", msg)
  lunit_do_assert(not not string.find(actual, pattern), "expected '"..actual.."' to match pattern '"..pattern.."' but doesn't", msg)
  return actual
end

function assert_not_match(pattern, actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_match", msg)
  lunit_do_assert(is_string(actual), "expected a string to not match pattern '"..pattern.."' but was a '"..type(actual).."'", msg)
  lunit_do_assert(string.find(actual, pattern) == nil, "expected '"..actual.."' to not match pattern '"..pattern.."' but it does", msg)
  return actual
end

function assert_nil(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_nil", msg)
  lunit_do_assert(is_nil(actual), "nil expected but was a "..type(actual), msg)
  return actual
end

function assert_not_nil(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_nil", msg)
  lunit_do_assert(not is_nil(actual), "nil not expected but was one", msg)
  return actual
end

function assert_boolean(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_boolean", msg)
  lunit_do_assert(is_boolean(actual), "boolean expected but was a "..type(actual), msg)
  return actual
end

function assert_not_boolean(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_boolean", msg)
  lunit_do_assert(not is_boolean(actual), "boolean not expected but was one", msg)
  return actual
end

function assert_number(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_number", msg)
  lunit_do_assert(is_number(actual), "number expected but was a "..type(actual), msg)
  return actual
end

function assert_not_number(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_number", msg)
  lunit_do_assert(not is_number(actual), "number not expected but was one", msg)
  return actual
end

function assert_string(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_string", msg)
  lunit_do_assert(is_string(actual), "string expected but was a "..type(actual), msg)
  return actual
end

function assert_not_string(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_string", msg)
  lunit_do_assert(not is_string(actual), "string not expected but was one", msg)
  return actual
end

function assert_table(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_table", msg)
  lunit_do_assert(is_table(actual), "table expected but was a "..type(actual), msg)
  return actual
end

function assert_not_table(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_table", msg)
  lunit_do_assert(not is_table(actual), "table not expected but was one", msg)
  return actual
end

function assert_function(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_function", msg)
  lunit_do_assert(is_function(actual), "function expected but was a "..type(actual), msg)
  return actual
end

function assert_not_function(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_function", msg)
  lunit_do_assert(not is_function(actual), "function not expected but was one", msg)
  return actual
end

function assert_thread(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_thread", msg)
  lunit_do_assert(is_thread(actual), "thread expected but was a "..type(actual), msg)
  return actual
end

function assert_not_thread(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_thread", msg)
  lunit_do_assert(not is_thread(actual), "thread not expected but was one", msg)
  return actual
end

function assert_userdata(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_userdata", msg)
  lunit_do_assert(is_userdata(actual), "userdata expected but was a "..type(actual), msg)
  return actual
end

function assert_not_userdata(actual, msg)
  lunit_stats_inc("assertions")
  lunit_check_msg("assert_not_userdata", msg)
  lunit_do_assert(not is_userdata(actual), "userdata not expected but was one", msg)
  return actual
end

function assert_error(msg, func)
  lunit_stats_inc("assertions")
  if is_nil(func) then func, msg = msg, nil end
  lunit_check_msg("assert_error", msg)
  lunit_do_assert(is_function(func), "assert_error expects a function as the last argument but it was a "..type(func))
  local ok, errmsg = pcall(func)
  lunit_do_assert(ok == false, "error expected but no error occurred", msg)
end

function assert_pass(msg, func)
  lunit_stats_inc("assertions")
  if is_nil(func) then func, msg = msg, nil end
  lunit_check_msg("assert_pass", msg)
  lunit_do_assert(is_function(func), "assert_pass expects a function as the last argument but it was a "..type(func))
  local ok, errmsg = pcall(func)
  if not ok then lunit_do_assert(ok == true, "no error expected but error was: "..errmsg, msg) end
end


-----------------------------------------------------------
-- Assert implementation that assumes it was called from --
-- lunit code which was called directly from user code.  --
-----------------------------------------------------------
function lunit_do_assert(assertion, base_msg, user_msg)
  assert(is_boolean(assertion))
  assert(is_string(base_msg))
  assert(is_string(user_msg) or is_nil(user_msg))
  if not assertion then
    if user_msg then
      error(base_msg..": "..user_msg, 3)
    else
      error(base_msg.."!", 3)
    end
  end
end

-------------------------------------------
-- Checks the msg argument in assert_xxx --
-------------------------------------------
function lunit_check_msg(name, msg)
  assert(is_string(name))
  if not (is_nil(msg) or is_string(msg)) then
    error("lunit."..name.."() expects the optional message as a string but it was a "..type(msg).."!" ,3)
  end
end


-------------------------------------
-- Creates a new TestCase 'Object' --
-------------------------------------
function lunit.TestCase(name)
  lunit_do_assert(is_string(name), "lunit.TestCase() needs a string as an argument")
  local tc = {
    __lunit_name = name;
    __lunit_setup = nil;
    __lunit_tests = { };
    __lunit_teardown = nil;
  }
  setmetatable(tc, lunit.tc_mt)
  table.insert(lunit.testcases, tc)
  return tc
end

lunit.tc_mt = {
  __newindex = function(tc, key, value)
    rawset(tc, key, value)
    if is_string(key) and is_function(value) then
      local name = string.lower(key)
      if string.find(name, "^test") or string.find(name, "test$") then
        table.insert(tc.__lunit_tests, key)
      elseif name == "setup" then
        tc.__lunit_setup = value
      elseif name == "teardown" then
        tc.__lunit_teardown = value
      end
    end
  end
}

-----------------------------------------
-- Wrap Functions in a TestCase object --
-----------------------------------------
function wrap(name, ...)
  if is_function(name) then
    table.insert(arg, 1, name)
    name = "Anonymous Testcase"
  end

  local tc = lunit.TestCase(name)
  for index, test in ipairs(arg) do
    tc["Test #"..tostring(index)] = test
  end
  return tc
end

----------------------------------
-- Runs the complete Test Suite --
----------------------------------
function lunit.run()
  ---------------------------
  -- Initialize statistics --
  ---------------------------
  lunit.stats.ntestcases = 0	-- Total number of Test Cases
  lunit.stats.tests = 0	-- Total number of all Tests in all Test Cases
  lunit.stats.run = 0		-- Number of Tests run
  lunit.stats.notrun = 0	-- Number of Tests not run
  lunit.stats.failed = 0	-- Number of Tests failed
  lunit.stats.passed = 0	-- Number of Test passed
  lunit.stats.assertions = 0	-- Number of all assertions made in all Test in all Test Cases

  --------------------------------
  -- Count Test Cases and Tests --
  --------------------------------
  lunit.stats.ntestcases = #lunit.testcases

  for _, tc in ipairs(lunit.testcases) do
    lunit_stats_inc("tests" , #tc.__lunit_tests)
  end

  ------------------
  -- Print Header --
  ------------------
  print()
  print("#### Test Suite with "..lunit.stats.tests.." Tests in "..lunit.stats.ntestcases.." Test Cases loaded.")

  ------------------------
  -- Run all Test Cases --
  ------------------------
  for _, tc in ipairs(lunit.testcases) do
    lunit_run_testcase(tc)
  end

  ------------------
  -- Print Footer --
  ------------------
  print()
  print("#### Test Suite finished.")

  local msg_assertions = lunit.stats.assertions.." Assertions checked. "
  local msg_passed     = lunit.stats.passed == lunit.stats.tests and "All Tests passed" or  lunit.stats.passed.." Tests passed"
  local msg_failed     = lunit.stats.failed > 0 and ", "..lunit.stats.failed.." failed" or ""
  local msg_run	       = lunit.stats.notrun > 0 and ", "..lunit.stats.notrun.." not run" or ""

  print()
  print(msg_assertions..msg_passed..msg_failed..msg_run.."!")

  -----------------
  -- Return code --
  -----------------
  if lunit.stats.passed == lunit.stats.tests then
    return 0
  else
    return 1
  end
end

-----------------------------
-- Runs a single Test Case --
-----------------------------
function lunit_run_testcase(tc)
  assert(is_table(tc))
  assert(is_table(tc.__lunit_tests))
  assert(is_string(tc.__lunit_name))
  assert(is_nil(tc.__lunit_setup) or is_function(tc.__lunit_setup))
  assert(is_nil(tc.__lunit_teardown) or is_function(tc.__lunit_teardown))

  --------------------------------------------
  -- Protected call to a Test Case function --
  --------------------------------------------
  local function call(errprefix, func)
    assert(is_string(errprefix))
    assert(is_function(func))
    local ok, errmsg = xpcall(function() func(tc) end, debug.traceback)
    if not ok then
      print()
      print(errprefix..": "..errmsg)
    end
    return ok
  end

  ------------------------------------
  -- Calls setup() on the Test Case --
  ------------------------------------
  local function setup()
    if tc.__lunit_setup then
      return call("ERROR: setup() failed", tc.__lunit_setup)
    else
      return true
    end
  end

  ------------------------------------------
  -- Calls a single Test on the Test Case --
  ------------------------------------------
  local function run(testname)
    assert(is_string(testname))
    assert(is_function(tc[testname]))
    local ok = call("FAIL: "..testname, tc[testname])
    if not ok then
      lunit_stats_inc("failed")
    else
      lunit_stats_inc("passed")
    end
    return ok
  end

  ---------------------------------------
  -- Calls teardown() on the Test Case --
  ---------------------------------------
  local function teardown()
     if tc.__lunit_teardown then
       call("WARNING: teardown() failed", tc.__lunit_teardown)
     end
  end

  ---------------------------------
  -- Run all Tests on a TestCase --
  ---------------------------------
  print()
  print("#### Running '"..tc.__lunit_name.."' ("..#tc.__lunit_tests.." Tests)...")

  for _, testname in ipairs(tc.__lunit_tests) do
    if setup() then
      run(testname)
      lunit_stats_inc("run")
      teardown()
    else
      print("WARN: Skipping '"..testname.."'...")
      lunit_stats_inc("notrun")
    end
  end
end

--------------------------------------------------
-- Increments a counter in the statistics table --
--------------------------------------------------
function lunit_stats_inc(varname, value)
  assert(is_table(lunit.stats))
  assert(is_string(varname))
  assert(is_nil(value) or is_number(value))
  if not lunit.stats[varname] then return end
  lunit.stats[varname] = lunit.stats[varname] + (value or 1)
end
