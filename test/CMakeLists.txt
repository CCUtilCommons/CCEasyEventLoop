message(STATUS "Creating the emit_sync_test for the sync signal test")
add_executable(emit_sync_test emit_sync_test.cpp)
target_link_libraries(emit_sync_test CoreLibrary)

add_executable(emit_async_test emit_async_test.cpp)
target_link_libraries(emit_async_test CoreLibrary)

add_executable(timer_test timer_test.cpp)
target_link_libraries(timer_test CoreLibrary)