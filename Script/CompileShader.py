import argparse
import os

parser = argparse.ArgumentParser(description='GLSL shader compiler.')
parser.add_argument('--target', required=True, help='Compile target GLSL shader file.')

args = parser.parse_args()

shader_compile_target = args.target
shader_compiler_path = "glslc.exe"
shader_compile_option = "-o" # https://github.com/google/shaderc/tree/main/glslc#compilation-stage-selection-options
shader_compile_output = shader_compile_target + ".spv"

os.system(f'{shader_compiler_path} \"{shader_compile_target}\" {shader_compile_option} \"{shader_compile_output}\"')