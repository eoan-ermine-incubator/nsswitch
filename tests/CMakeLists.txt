enable_testing()
include(GoogleTest)

set(NSSWITCH_TEST_SOURCES
    altlinux_man
    freeBSD_man
    linux_man
    sberlinux_man
    tldp
)

foreach(source IN LISTS NSSWITCH_TEST_SOURCES)
  add_executable(nsswitch_${source}_test nsswitch_${source}.cpp)
  target_link_libraries(nsswitch_${source}_test PRIVATE
      GTest::gtest_main
      nsswitch
  )

  gtest_discover_tests(nsswitch_${source}_test)
endforeach()