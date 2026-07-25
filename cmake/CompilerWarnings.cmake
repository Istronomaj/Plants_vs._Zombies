# Applies the project's warning set to one of OUR targets.
#
# Deliberately a function taking an explicit target: the previous version ran
# at include time against ${PROJECT_NAME} and reached into SFML's own targets
# by hardcoded name, which both coupled us to SFML's internals and applied our
# flags to third-party code.
function(pvz_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
                /W4
                /permissive-
                /utf-8
                /MP
                /external:anglebrackets
                /external:W0
        )
    else()
        target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wshadow
                -Wnon-virtual-dtor
                -Wold-style-cast
                -Wcast-align
                -Woverloaded-virtual
                -Wconversion
                -Wsign-conversion
                -Wdouble-promotion
                -Wformat=2
        )
    endif()

    if(PVZ_WARNINGS_AS_ERRORS)
        set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
    endif()
endfunction()
