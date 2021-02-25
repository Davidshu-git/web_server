add_executable(atomic_unittest Atomic_unittest.cc)
add_test(NAME atomic_unittest COMMAND atomic_unittest)

add_executable(atomic_effective Atomic_effective.cc)
target_link_libraries(atomic_effective pthread)
add_test(NAME atomic_effective COMMAND atomic_effective)

add_executable(current_thread_unittest CurrentThread_unittest.cc)
target_link_libraries(current_thread_unittest base_lib)
add_test(NAME current_thread_unittest COMMAND current_thread_unittest)

add_executable(mutex_test Mutex_test.cc)
add_test(NAME mutex_test COMMAND mutex_test)

add_executable(thread_test Thread_test.cc)
target_link_libraries(thread_test base_lib)
add_test(NAME thread_test COMMAND thread_test)

add_executable(timestamp_unittest Timestamp_unittest.cc)
target_link_libraries(timestamp_unittest base_lib)
add_test(NAME timestamp_unittest COMMAND timestamp_unittest)

add_executable(logging_test Logging_test.cc)
target_link_libraries(logging_test base_lib)

add_executable(threadpool_test ThreadPool_test.cc)
target_link_libraries(threadpool_test base_lib)