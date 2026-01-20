set(net_SRCS
  acceptor.cpp
  buffer.cpp
  channel.cpp
  event_loop.cpp
  event_loop_thread.cpp
  epoller.cpp
  inet_address.cpp
  socket.cpp
  socket_utils.cpp
  timer_queue.cpp
  timer.cpp
  tcp_connection.cpp
  tcp_server.cpp
  )

add_library(mini_muduo_net ${net_SRCS})
target_link_libraries(mini_muduo_net mini_muduo_base)
install(TARGETS mini_muduo_net DESTINATION lib)

set(HEADERS
  acceptor.h
  buffer.h
  channel.h
  event_loop.h
  event_loop_thread.h
  epoller.h
  inet_address.h
  socket.h
  socket_utils.h
  timer.h
  timer_queue.h
  tcp_connection.h
  tcp_server.h
  callbacks.h
  )

install(FILES ${HEADERS} DESTINATION include/mini_muduo/net)

if (MUDUO_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif ()