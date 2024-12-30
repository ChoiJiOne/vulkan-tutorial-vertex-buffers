cmake_minimum_required(VERSION 3.27)

add_subdirectory(1.3.1.Introduction)
add_subdirectory(1.3.2.ShaderModules)
add_subdirectory(1.3.3.FixedFunctions)
add_subdirectory(1.3.4.RenderPasses)

set_target_properties(
    1.3.1.Introduction
    1.3.2.ShaderModules
    1.3.3.FixedFunctions
    1.3.4.RenderPasses
    PROPERTIES FOLDER "1.Drawing a triangle/1.3.Graphics pipeline basics"
)