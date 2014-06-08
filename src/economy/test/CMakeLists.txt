wl_test(test_economy
  SRCS
    ./economy_test_main.cc
    #TODO: (code review): are these three tests all run even though we only add one test at the end?
    ./test_economy.cc
    ./test_road.cc
    ./test_routing.cc
  DEPENDS
    widelands_all
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${TEST_EXTRA_LIBS}
)
