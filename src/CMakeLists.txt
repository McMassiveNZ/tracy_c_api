set(current_target tracy-c-api-test)

add_executable(
    ${current_target} 
    main.cpp
    profiler.h
    profiler.cpp
)

CPMAddPackage(
  NAME tracy
  GITHUB_REPOSITORY wolfpld/tracy
  GIT_TAG v0.9.1
  VERSION 0.9.1
  OPTIONS
    "TRACY_ON_DEMAND"
)

target_link_libraries(${current_target} PRIVATE TracyClient)
set_property(TARGET TracyClient PROPERTY FOLDER third_party/tracy)