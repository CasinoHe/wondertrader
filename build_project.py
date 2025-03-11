# build dependent thirdparties and build project

import argparse
import sys
import os
import subprocess
import shutil

BUILD_DEPENDENCIES = "build-dependencies"
BUILD_PROJECT = "build-project"
VCPKG_PATH = "thirdparties/vcpkg"


class BuildBase(object):
    def __init__(self, args, parser=None):
        self.args = args
        self.parser = parser

        self.init_project_path()

    def init_project_path(self):
        self.project_root = os.path.abspath(os.path.dirname(__file__))
        self.src_root = os.path.abspath(os.path.join(self.project_root, "src"))
        self.vcpkg_dir = os.path.abspath(os.path.abspath(os.path.join(os.path.dirname(__file__), VCPKG_PATH)))

    def _run_command(self, workdir, command, args, env=None):
        if not os.path.exists(workdir):
            raise RuntimeError("Runcommand %s Workdir not found: %s" % (command, workdir))

        cwd = os.getcwd()
        os.chdir(workdir)
        if sys.platform == "win32":
            command_list = ["cmd.exe", "/c", command] + args
        else:
            command_list = [command] + args

        print(f"Starting Run command: {command_list} under {workdir}")
        process = subprocess.Popen(command_list, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, text=True, shell=False, env=env)

        while True:
            output = process.stdout.readline()
            if output == '' and process.poll() is not None:
                break
            if output:
                print(output.strip())

        stderr_output = process.stderr.read()
        os.chdir(cwd)
        if process.returncode != 0:
            print("Failed to run command: %s %s" % (command, " ".join(args)))
            print("stderr: %s" % stderr_output)
            return False
        else:
            print("Run command success: %s %s" % (command, " ".join(args)))
            return True

    def get_triplet(self):
        if self.args.triplet:
            return self.args.triplet
        else:
            return "x64-windows-static-release" if sys.platform == "win32" else "x64-linux-release"

    def build(self):
        raise NotImplementedError("Build method not implemented")


class BuildDependencies(BuildBase):
    def __init__(self, args, parser=None):
        super(__class__, self).__init__(args, parser)

    def init_project_path(self):
        super(__class__, self).init_project_path()
        # detect platform
        if sys.platform == "win32":
            self.vcpkg_path = os.path.join(self.vcpkg_dir, "vcpkg.exe")
        else:
            self.vcpkg_path = os.path.join(self.vcpkg_dir, "vcpkg")

    def prepare_vcpkg(self):
        if not os.path.exists(self.vcpkg_dir):
            raise RuntimeError("Vcpkg path not found: %s" % self.vcpkg_dir)

        print("Build dependencies with vcpkg path: %s" % self.vcpkg_dir)

        def build_vcpkg():
            if sys.platform == "win32":
                return self._run_command(self.vcpkg_dir, "bootstrap-vcpkg.bat", [])
            else:
                return self._run_command(self.vcpkg_dir, "bootstrap-vcpkg.sh", [])

        if not os.path.exists(self.vcpkg_path):
            # compile vcpkg
            if not build_vcpkg():
                raise RuntimeError("Failed to compile vcpkg")
        else:
            if self.args.force_rebuild:
                # update vcpkg
                if not build_vcpkg():
                    raise RuntimeError("Failed to update vcpkg")
            else:
                print("Vcpkg already exists, skip build")

    def build_dependencies(self):
        if not os.path.exists(self.vcpkg_path):
            raise RuntimeError("Vcpkg path not found: %s" % self.vcpkg_path)

        triplet = self.get_triplet()

        # set build variant
        env = os.environ.copy()
        if self.args.build_variant == "Release":
            env["VCPKG_BUILD_TYPE"] = "Release"
        elif self.args.build_variant == "Debug":
            env["VCPKG_BUILD_TYPE"] = "Debug"

        args = ["install", "--triplet", triplet, "--vcpkg-root", self.vcpkg_dir,
                "--x-install-root", os.path.join(self.project_root, "build", "vcpkg_installed"),
                "--clean-buildtrees-after-build", "--clean-packages-after-build"]
        if self.args.dry_run:
            args.append("--dry-run")
        if not self._run_command(self.src_root, self.vcpkg_path, args, env=env):
            raise RuntimeError("Failed to install dependencies")

        print(f"Build dependencies for quanttrader {self.args.build_variant} finished.")

    def build(self):
        self.prepare_vcpkg()
        self.build_dependencies()


class BuildProject(BuildBase):
    def __init__(self, args, parser=None):
        super(__class__, self).__init__(args, parser)

    def configure_project(self):
        print("Start building project...")

        triplet = self.get_triplet()

        if self.args.enable_install:
            manifest_install = "ON"
        else:
            if not os.path.exists(os.path.join(self.project_root, "build", "vcpkg_installed")):
                manifest_install = "ON"
            else:
                manifest_install = "OFF"

        variant = self.args.build_variant

        args = [
            f"-DCMAKE_TOOLCHAIN_FILE={self.vcpkg_dir}/scripts/buildsystems/vcpkg.cmake",
            f"-DVCPKG_MANIFEST_ROOT={self.src_root}",
            f"-DVCPKG_TARGET_TRIPLET={triplet}",
            f"-DVCPKG_MANIFEST_INSTALL={manifest_install}",
            f"-DCMAKE_BUILD_TYPE={variant}",
            f"{self.src_root}"
        ]

        workdir = os.path.join(self.project_root, "build")
        if not os.path.exists(workdir):
            os.makedirs(workdir)

        command = "cmake"
        if not self._run_command(workdir, command, args):
            raise RuntimeError("Failed to install dependencies")

        return True

    def build_project(self):
        print(f"Start building project using {self.args.build_variant}.")
        workdir = os.path.join(self.project_root, "build")
        if not os.path.exists(workdir):
            raise RuntimeError("Workdir not found: %s" % workdir)

        args = ["--build", ".", "--config", self.args.build_variant]
        if not self._run_command(workdir, "cmake", args):
            raise RuntimeError("Failed to build project.")

    def clean_build(self):
        workdir = os.path.join(self.project_root, "build")
        if os.path.exists(workdir):
            print(f"Remove build directory: {workdir}")
            shutil.rmtree(workdir)

        return True

    def build(self):
        if self.args.clean:
            self.clean_build()
        else:
            self.configure_project()
            self.build_project()


def parse_args(args):
    parser = argparse.ArgumentParser(description="Build project", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    sub_parser = parser.add_subparsers(help="sub-command help", dest="command")

    build_dependencies_parser = sub_parser.add_parser(BUILD_DEPENDENCIES, help="Build dependencies")
    build_dependencies_parser.add_argument("--build-variant", default="Release", choices=["Release", "Debug"], help="Build type")
    build_dependencies_parser.add_argument("--force-rebuild", action="store_true", default=False, help="Force rebuild all dependencies")
    build_dependencies_parser.add_argument("--triplet", type=str, required=False, help="Set vcpkg triplet, example: x64-windows-static")
    build_dependencies_parser.add_argument("--dry-run", action="store_true", default=False, help="Use vcpkg on --dry-run mode")

    project_parser = sub_parser.add_parser(BUILD_PROJECT, help="Build project")
    project_parser.add_argument("--build-variant", default="Release", choices=["Release", "Debug"], help="Build type")
    project_parser.add_argument("--disable-vcpkg", action="store_true", default=False, help="Disable vcpkg use system libraries")
    project_parser.add_argument("--enable-install", action="store_true", default=False, help="Enable auto install vcpkg dependencies")
    project_parser.add_argument("--triplet", type=str, required=False, help="Set vcpkg triplet, example: x64-windows-static")
    project_parser.add_argument("--clean", action="store_true", default=False, help="Clean build directory")

    return parser.parse_args(args), parser


def build_all():
    # build dependencies first
    sys.argv.append(BUILD_DEPENDENCIES)
    args, parser = parse_args(None)
    build_dependencies = BuildDependencies(args, parser)
    build_dependencies.build()

    # build project then
    sys.argv[1] = BUILD_PROJECT
    args, parser = parse_args(None)
    build_project = BuildProject(args, parser)
    build_project.build()


def build_according_to_arguments():
    args, parser = parse_args(None)

    if args.command == BUILD_DEPENDENCIES:
        build_dependencies = BuildDependencies(args, parser)
        build_dependencies.build()
    elif args.command == BUILD_PROJECT:
        build_project = BuildProject(args, parser)
        build_project.build()
    else:
        parser.print_help()
        sys.exit(1)


def main():
    # if there is no arguments send to python, ask user to input Y/N to build all
    if len(sys.argv) == 1:
        print("Do you want to build all? Y/N")
        user_input = input()
        user_input = user_input.lower()
        if user_input == "y" or user_input == "yes":
            build_all()
        else:
            build_according_to_arguments()
    else:
        build_according_to_arguments()


if __name__ == "__main__":
    main()
