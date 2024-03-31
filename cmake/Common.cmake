#[[
    The add_flags function takes a list and a variable name, and appends
    the flags to the list. The flags are passed as arguments to the function.
    The function skips empty arguments.
]]
function(add_flags VAR)
    foreach (FLAG ${ARGN}) # Skip empty arguments.
        message(DEBUG "Adding flag ${FLAG} to ${VAR}")
        list(APPEND ${VAR} ${FLAG})
    endforeach ()
    set(${VAR} "${${VAR}}" PARENT_SCOPE)
endfunction()

