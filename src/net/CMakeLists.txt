set(net_SRCS
  channel.cpp
  event_loop.cpp
  event_loop_thread.cpp
  epoller.cpp
  timer_queue.cpp
  timer.cpp
  )

add_library(mini_muduo_net ${net_SRCS})
target_link_libraries(mini_muduo_net mini_muduo_base)
install(TARGETS mini_muduo_net DESTINATION lib)

set(HEADERS
  channel.h
  eventloop.h
  epoller.h
  )

install(FILES ${HEADERS} DESTINATION include/mini_muduo/net)

if (MUDUO_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif ()