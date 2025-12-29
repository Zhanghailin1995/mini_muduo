set(base_SRCS
  current_thread.cpp
  condition.cpp
  mutex.h 
  noncopyable.h
)


add_library(mini_muduo_base ${base_SRCS})
target_link_libraries(mini_muduo_base pthread rt)

# # 定义一个头文件库
# add_library(mini_muduo_base INTERFACE)

# # 添加头文件路径到目标
# target_include_directories(mini_muduo_base INTERFACE ${BASE_INCLUDE_DIR})

# # 链接依赖库
# target_link_libraries(mini_muduo_base INTERFACE pthread rt)

install(TARGETS mini_muduo_base DESTINATION lib)
file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/mini_muduo/base)

if (MUDUO_BUILD_EXAMPLES)
  add_subdirectory(tests)
endif ()