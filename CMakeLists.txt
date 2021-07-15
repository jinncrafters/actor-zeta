cmake_minimum_required(VERSION 3.0)

PROJECT(actor-zeta CXX)

set(CMAKE_CXX_STANDARD 11 CACHE STRING "The C++ standard whose features are requested." )
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message (STATUS "STANDARD CPP = ${CMAKE_CXX_STANDARD}")

message (STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

option(ALLOW_EXAMPLES "ALLOW EXAMPLES" OFF)

option(ALLOW_TESTS "ALLOW TESTS" OFF)

option(RTTI_DISABLE "RTTI DISABLE" ON)

option(EXCEPTIONS_DISABLE "EXCEPTIONS DISABLE" ON)

message(STATUS "ALLOW_EXAMPLES = ${ALLOW_EXAMPLES}")
message(STATUS "ALLOW_TESTS = ${ALLOW_TESTS}")
message(STATUS "RTTI_DISABLE = ${RTTI_DISABLE}")
message(STATUS "EXCEPTIONS_DISABLE = ${EXCEPTIONS_DISABLE}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    if (EXCEPTIONS_DISABLE)
        message("Disable C++ exceptions.")
        string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    endif()

    if (RTTI_DISABLE)
        message("Disable RTTI.")
        string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
    endif()

else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    if (EXCEPTIONS_DISABLE)
        message("Disable C++ exceptions.")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    endif()

    if (RTTI_DISABLE)
        message("Disable RTTI.")
        string(REGEX REPLACE "-frtti" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    endif()

endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")


if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    add_compile_options(
            -Wall
            -Wextra
            -Wpedantic

            -Wcast-align
            -Wcast-qual
            -Wconversion
            -Wctor-dtor-privacy
            -Wenum-compare
            -Wfloat-equal
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Woverloaded-virtual
            -Wredundant-decls
            -Wsign-conversion
            -Wsign-promo
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /WX)
endif()

include(cmake/clang_tools.cmake)

include_directories(SYSTEM .)
include_directories(SYSTEM header)

set(HEADER_FILES
        header/actor-zeta/base/actor_abstract.hpp
        header/actor-zeta/base/actor.hpp
        header/actor-zeta/base/address.hpp
        header/actor-zeta/base/cooperative_actor.hpp
        header/actor-zeta/base/basic_actor.hpp
        header/actor-zeta/base/communication_module.hpp
        header/actor-zeta/base/handler.hpp
        header/actor-zeta/base/supervisor.hpp
        header/actor-zeta/base/supervisor_abstract.hpp

        header/actor-zeta/detail/pmr/container.hpp
        header/actor-zeta/detail/pmr/memory_resource.hpp
        header/actor-zeta/detail/any.hpp
        header/actor-zeta/detail/callable_trait.hpp
        header/actor-zeta/detail/intrusive_ptr.hpp
        header/actor-zeta/detail/ref_counted.hpp
        header/actor-zeta/detail/string_view.hpp
        header/actor-zeta/detail/type_list.hpp
        header/actor-zeta/detail/type_traits.hpp

        header/actor-zeta/executor/policy/profiled.hpp
        header/actor-zeta/executor/policy/unprofiled.hpp
        header/actor-zeta/executor/policy/work_sharing.hpp
        header/actor-zeta/executor/abstract_executor.hpp
        header/actor-zeta/executor/executor.hpp
        header/actor-zeta/executor/executable.hpp
        header/actor-zeta/executor/profiled_executor.hpp
        header/actor-zeta/executor/execution_device.hpp
        header/actor-zeta/executor/worker.hpp

        header/actor-zeta/base/message.hpp

        header/actor-zeta/forwards.hpp
        header/actor-zeta/link.hpp
        header/actor-zeta/send.hpp
        header/actor-zeta.hpp
)

set(SOURCE_FILES

        source/actor/actor_abstract.cpp
        source/actor/actor.cpp
        source/actor/address.cpp
        source/actor/cooperative_actor.cpp
        source/actor/supervisor.cpp
        source/actor/supervisor_abstract.cpp
        source/actor/communication_module.cpp

        source/detail/pmr/memory_resource.cpp
        source/detail/string_view.cpp
        source/detail/ref_counted.cpp

        source/executor/abstract_executor.cpp
        source/executor/executable.cpp
        source/executor/execution_device.cpp
        source/executor/work_sharing.cpp


        source/messaging/message.cpp
        source/src.cpp
        source/send.cpp
)


find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)


find_package( Threads )

add_library(
        ${PROJECT_NAME}
        ${SOURCE_FILES}
        ${HEADER_FILES}
)


target_link_libraries (
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_THREAD_LIBS_INIT}
)


target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/header/actor-zeta
        ${CMAKE_CURRENT_SOURCE_DIR}/header
)

if(ALLOW_EXAMPLES)
    add_subdirectory(examples)
endif()

if (ALLOW_TESTS)
    # configuration
    #############################################################################################
    #conan

    message("CONAN_BUILD_INFO_DIR = ${CMAKE_BINARY_DIR}")
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS)
    message(STATUS "LIBS = ${CONAN_LIBS}")
    message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

    # conan
    #############################################################################################
    enable_testing()
    add_subdirectory(test)
endif ()
