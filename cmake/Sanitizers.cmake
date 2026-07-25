# Enables AddressSanitizer + UndefinedBehaviorSanitizer on a target.
#
# Only applied to our own targets, and only when PVZ_ENABLE_SANITIZERS is set.
# The previous version also carried libc++ detection and an Apple libdispatch
# workaround; neither is our concern.
function(pvz_enable_sanitizers target)
    if(NOT PVZ_ENABLE_SANITIZERS)
        return()
    endif()

    if(MSVC)
        target_compile_options(${target} PRIVATE /fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(_pvz_san_flags -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_compile_options(${target} PRIVATE ${_pvz_san_flags})
        target_link_options(${target} PRIVATE ${_pvz_san_flags})
    endif()
endfunction()
