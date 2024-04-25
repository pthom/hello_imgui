import logging
import subprocess
import platform
import os
import sys
import shutil


THIS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_DIR = os.path.abspath(os.path.join(THIS_DIR, "..", ".."))

FAILING_TESTS_MESSAGES = []
SUCCESSES_RUN_APP = []
SUCCESSES_BUILD_APP = []
SUCCESSES_BUILD_VCPKG = []


def copy_mesa_libs_to_current_dir():
    # Test if we are on a github runner
    if "GITHUB_WORKFLOW" not in os.environ:
        return

    mesa3d_dll_dir = os.path.join(REPO_DIR, "mesa3d/x64")
    current_dir = os.getcwd()
    release_build_dir = os.path.join(current_dir, "Release")
    if not os.path.exists(release_build_dir):
        os.mkdir(release_build_dir)
    for file in os.listdir(mesa3d_dll_dir):
        print(f"Copying {file} to {release_build_dir}")
        shutil.copy(os.path.join(mesa3d_dll_dir, file), release_build_dir)


def run_test_with_rendering_backend(rendering_backend: str) -> bool:
    platform_backends = ["SDL", "GLFW"]

    print("Running test with rendering backend: " + rendering_backend)
    print("-------------------------------------------------")
    build_dir = f"build-{rendering_backend}"
    os.chdir(THIS_DIR)
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    os.mkdir(build_dir)
    os.chdir(build_dir)
    current_dir = os.getcwd()

    if platform.system() == "Windows":
        copy_mesa_libs_to_current_dir()

    def find_vcpkg_program() -> str:
        possible_vcpkg_roots = [f"{REPO_DIR}/vcpkg"]
        if "VCPKG_ROOT" in os.environ:
            print(f"Found VCPKG_ROOT in environment variables: {os.environ['VCPKG_ROOT']}")
            possible_vcpkg_roots.append(os.environ["VCPKG_ROOT"])
        if "VCPKG_INSTALLATION_ROOT" in os.environ:
            logging.info(f"Found VCPKG_INSTALLATION_ROOT in environment variables: {os.environ['VCPKG_INSTALLATION_ROOT']}")
            possible_vcpkg_roots.append(os.environ["VCPKG_INSTALLATION_ROOT"])

        r = ""
        for possible_vcpkg_root in possible_vcpkg_roots:
            possible_vcpkg_programs = [f"{possible_vcpkg_root}/vcpkg", f"{possible_vcpkg_root}/vcpkg.exe"]
            for possible_vcpkg_program in possible_vcpkg_programs:
                if os.path.isfile(possible_vcpkg_program):
                    r = possible_vcpkg_program
                    print(f"Found vcpkg program at {r}")
                    break
                if len(r) > 0:
                    break
        if len(r) == 0:
            logging.error(f"Could not find vcpkg program, tried {possible_vcpkg_roots}")
            sys.exit(1)
        return r

    vcpkg_program = find_vcpkg_program()

    if "VCPKG_ROOT" in os.environ:
        vcpkg_root = os.environ["VCPKG_ROOT"]
    elif "VCPKG_INSTALLATION_ROOT" in os.environ:
        vcpkg_root = os.environ["VCPKG_INSTALLATION_ROOT"]
    else:
        vcpkg_root = f"{REPO_DIR}/vcpkg"
    vcpkg_program = os.path.join(vcpkg_root, "vcpkg")

    if platform.system() == "Linux":
        os.environ["DISPLAY"] = ":99.0"

    overlay_dir = f"{REPO_DIR}/hello_imgui_cmake/overlay_vcpkg/hello-imgui"
    cmake_toolchain_file = os.path.join(vcpkg_root, "scripts/buildsystems/vcpkg.cmake")

    vcpkg_remove_cmd = f'{vcpkg_program} remove hello-imgui --recurse'
    vcpkg_install_cmd = f'{vcpkg_program} install --head hello-imgui[{rendering_backend},glfw-binding,sdl2-binding] --overlay-ports={overlay_dir} --recurse'
    cmake_cmd = f"cmake -DCMAKE_TOOLCHAIN_FILE={cmake_toolchain_file} .."
    build_cmd = f"cmake --build . --config Release"

    if platform.system() == "Windows":
        test_app_glfw_cmd = "Release/ci_vcpkg_package_tests_app.exe glfw"
        test_app_sdl_cmd = "Release/ci_vcpkg_package_tests_app.exe sdl"
    else:
        test_app_glfw_cmd = "./ci_vcpkg_package_tests_app glfw"
        test_app_sdl_cmd = "./ci_vcpkg_package_tests_app sdl"

    success = True
    cmds = {
        "vcpkg remove hello_imgui": vcpkg_remove_cmd,
        f"vcpkg install hello_imgui {rendering_backend}": vcpkg_install_cmd,
        f"cmake {rendering_backend}": cmake_cmd,
        f"build {rendering_backend}": build_cmd,
    }

    # Only run app with selected platform backends
    is_vulkan = rendering_backend == "experimental-vulkan-binding"
    is_macos = platform.system() == "Darwin"
    is_dx12_on_windows = rendering_backend == "experimental-dx12-binding" and platform.system() == "Windows"
    if not (is_vulkan or is_macos):
        cmds[f"test app (Glfw - {rendering_backend})"] = test_app_glfw_cmd
        if not is_dx12_on_windows:
            cmds[f"test app (SDL - {rendering_backend})"] = test_app_sdl_cmd

    for name, cmd in cmds.items():
        print(f"""
        ****************************************************************************************************
        Running {name} 
        Rendering backend: {rendering_backend}
        Command:
        {cmd}
        ****************************************************************************************************
        """)
        try:
            cmd_list = cmd.split()

            if platform.system() == "Windows" and name.startswith("test app"):
                # Under windows, the app succeeds, but the process returns an error code at exit
                # So, we detect if the app succeeded by checking if a file was created by it
                try:
                    subprocess.run(cmd_list, check=True, cwd=current_dir)
                except subprocess.CalledProcessError as e:
                    success_filename = "ci_vcpkg_package_test_app_success.txt"
                    if os.path.exists(success_filename):
                        os.remove(success_filename)
                    else:
                        raise e
            else:
                subprocess.run(cmd_list, check=True, cwd=current_dir)

            if name.startswith("test app"):
                SUCCESSES_RUN_APP.append(name)

            if name.startswith("build"):
                SUCCESSES_BUILD_APP.append(name)

            if name.startswith("vcpkg install"):
                SUCCESSES_BUILD_VCPKG.append(name)

        except subprocess.CalledProcessError as e:
            message = f"""
            ****************************************************************************************************
            Failed to run {name}
            Rendering backend: {rendering_backend}
            Command:
            {cmd}
            with error: 
            {e}
            ****************************************************************************************************
            """
            FAILING_TESTS_MESSAGES.append(message)
            print(message)
            success = False
    return success


def prepare_display():
    # Test if we are on a github runner
    if "GITHUB_WORKFLOW" not in os.environ:
        return
    if platform.system() == "Linux":
        subprocess.run("Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &", shell=True, check=True)


def rename_vcpkg_json():
    """We need to rename the vcpkg.json file at the repository root to avoid using it"""
    vcpkg_json_path = os.path.join(REPO_DIR, "vcpkg.json")
    vcpkg_json_backup_path = os.path.join(REPO_DIR, "vcpkg.json.bak")
    if os.path.exists(vcpkg_json_backup_path):
        os.remove(vcpkg_json_backup_path)
        print(f"Removed vcpkg.json.bak")
    if os.path.exists(vcpkg_json_path):
        os.rename(vcpkg_json_path, vcpkg_json_backup_path)
        print(f"Renamed vcpkg.json to vcpkg.json.bak")


def undo_rename_vcpkg_json():
    vcpkg_json_path = os.path.join(REPO_DIR, "vcpkg.json")
    vcpkg_json_backup_path = os.path.join(REPO_DIR, "vcpkg.json.bak")
    if os.path.exists(vcpkg_json_path):
        os.remove(vcpkg_json_path)
        print(f"Removed vcpkg.json")
    if os.path.exists(vcpkg_json_backup_path):
        os.rename(vcpkg_json_backup_path, vcpkg_json_path)
        print(f"Renamed vcpkg.json.bak to vcpkg.json")


def run_tests():
    rename_vcpkg_json()
    prepare_display()
    rendering_backends = ["opengl3-binding"]
    if platform.system() != "Darwin":
        rendering_backends.append("experimental-vulkan-binding")
        """
        Note: CI hangs on this step under macOS (spirv-tools)
        --
            Building spirv-tools:x64-osx@1.3.268.0...
            -- Downloading https://github.com/KhronosGroup/SPIRV-Tools/archive/vulkan-sdk-1.3.268.0.tar.gz -> KhronosGroup-SPIRV-Tools-vulkan-sdk-1.3.268.0.tar.gz...
            -- Extracting source /usr/local/share/vcpkg/downloads/KhronosGroup-SPIRV-Tools-vulkan-sdk-1.3.268.0.tar.gz
            -- Applying patch cmake-config-dir.diff
            -- Applying patch spirv-tools-shared.diff
            -- Applying patch fix-tool-deps.diff
            -- Using source at /usr/local/share/vcpkg/buildtrees/spirv-tools/src/-1.3.268.0-e268efb20d.clean
            -- Configuring x64-osx
            -- Building x64-osx-dbg
        """
    if platform.system() == "Windows":
        rendering_backends.append("experimental-dx11-binding")
        rendering_backends.append("experimental-dx12-binding")
    elif platform.system() == "Darwin":
        rendering_backends.append("metal-binding")

    all_success = True
    for backend in rendering_backends:
        success = run_test_with_rendering_backend(backend)
        all_success = all_success and success

    all_successes_run_str = "\n        ".join(SUCCESSES_RUN_APP)
    all_successes_build_app_str = "\n        ".join(SUCCESSES_BUILD_APP)
    all_successes_build_vcpkg_str = "\n        ".join(SUCCESSES_BUILD_VCPKG)
    print(f"""
    ****************************************************************************************************
    All tests finished
    Successful Vcpkg Builds:
        {all_successes_build_vcpkg_str}
    Successful App Builds:
        {all_successes_build_app_str}
    Successful App Runs: 
        {all_successes_run_str}
    ****************************************************************************************************
    """)
    if all_success:
        print("All tests passed")
    else:
        all_failures_str = "\n    ".join(FAILING_TESTS_MESSAGES)
        print(f"""
        !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        Some tests failed
        Failures:
        {all_failures_str}
        !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        """)

    undo_rename_vcpkg_json()
    if all_success:
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == "__main__":
    run_tests()
