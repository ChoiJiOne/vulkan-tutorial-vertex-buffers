import argparse
import os
from pathlib import Path

parser = argparse.ArgumentParser(description='GLSL shader compiler.')
parser.add_argument('--target', required=True, help='Compile target GLSL shader file.')
parser.add_argument('--output', required=True, help='Output GLSL shader file path.')

args = parser.parse_args()

shader_compiler_path = "glslc.exe"
shader_compile_option = "-o" # https://github.com/google/shaderc/tree/main/glslc#compilation-stage-selection-options

shader_compile_target = args.target
shader_file = os.path.splitext(os.path.basename(shader_compile_target))

shader_compile_output_path = args.output
shader_compile_output = f"{shader_compile_output_path}/{shader_file[0]}{shader_file[1]}.spv"

if not os.path.isdir(shader_compile_output_path):
    os.makedirs(shader_compile_output_path, exist_ok=False)

print(f">> COMPILE GLSL SHADER: {shader_compile_target} <<")
os.system(f'{shader_compiler_path} \"{shader_compile_target}\" {shader_compile_option} \"{shader_compile_output}\"')