# target_post_build_copy <file>... <destination>
function(target_post_build_copy target file destination)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${file} ${destination})
endfunction()