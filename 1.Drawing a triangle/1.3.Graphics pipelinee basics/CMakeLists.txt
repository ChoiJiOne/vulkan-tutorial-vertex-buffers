cmake_minimum_required(VERSION 3.27)

set(INTRODUCTION_PATH "1.3.1.Introduction")
set(SHADER_MODULES_PATH "1.3.2.Shader modules")
set(FIXED_FUNCTIONS_PATH "1.3.3.Fixed functions")
set(RENDER_PASSES_PATH "1.3.4.Render passes")
set(CONCLUSION_PATH "1.3.5.Conclusion")

set(INTRODUCTION "1.3.1.Introduction")
set(SHADER_MODULES "1.3.2.ShaderModules")
set(FIXED_FUNCTIONS "1.3.3.FixedFunctions")
set(RENDER_PASSES "1.3.4.RenderPasses")
set(CONCLUSION "1.3.5.Conclusion")

add_subdirectory(${INTRODUCTION_PATH})
add_subdirectory(${SHADER_MODULES_PATH})
add_subdirectory(${FIXED_FUNCTIONS_PATH})
add_subdirectory(${RENDER_PASSES_PATH})
add_subdirectory(${CONCLUSION_PATH})

set_target_properties(
    ${INTRODUCTION}
    ${SHADER_MODULES}
    ${FIXED_FUNCTIONS}
    ${RENDER_PASSES}
    ${CONCLUSION}
    PROPERTIES FOLDER "1.Drawing a triangle/1.3.Graphics pipeline basics"
)