# Patch Alembic's lib/Alembic/CMakeLists.txt to remove all install/export
# and packaging commands that conflict with FetchContent-provided Imath.
# Runs from Alembic's source dir (PATCH_COMMAND working directory).

set(_file "lib/Alembic/CMakeLists.txt")
set(_tmp "lib/Alembic/CMakeLists.txt.patched")

file(STRINGS "${_file}" _lines)
file(WRITE "${_tmp}" "")

# Lines to keep stop at the library target_include_directories block.
# Everything after is install/export/packaging.
set(_skip OFF)
foreach(_line IN LISTS _lines)
    if(_line MATCHES "^SET\\( ALEMBIC_LIB_INSTALL_DIR")
        set(_skip ON)
    endif()
    if(NOT _skip)
        file(APPEND "${_tmp}" "${_line}\n")
    endif()
endforeach()

file(RENAME "${_tmp}" "${_file}")
