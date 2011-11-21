import glob
import shutil
import os
import os.path
import subprocess
class basic_gxx_compiler:
    def __init__(self, cxxbinary):
        self.cxx = cxxbinary
    def do_compile(self, source, output, defines, include_directories):
        try:
            subprocess.check_output(
                [self.cxx]
                + list(map(lambda d: "-D" + d, defines))
                + list(map(lambda i: "-I" + i, include_directories))
                + ["-ftemplate-depth=128"]
                + ["-O3"]
                + ["-c"] + [source]
                + ["-o"] + [output], stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print(e.output)
            raise
    def do_link(self, sources, output, library_directories, libraries):
        try:
            subprocess.check_output(
                [self.cxx]
                + ["-s"]
                + list(map(lambda L: "-L" + L, library_directories))
                + ["-Wl,-subsystem,windows"]
                + ["-Xlinker", "--enable-auto-import"]
                + ["-o"] + [output]
                + sources
                + list(map(lambda l: "-l" + l, libraries)),
                stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print(e.output)
            raise

class windres:
    def __init__(self, windresbinary):
        self.windres = windresbinary
    def do_compile(self, input, output):
        try:
            subprocess.check_output(
                [self.windres]
                + [input]
                + ["-o"] + [output],
                stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print(e.output)
            raise

def build_binary(
    filenamegenerator,
    compiler, rc_compiler,
    sources, rc_file, output,
    defines, include_directories,
    library_directories, libraries):
    #compile sources into object files
    objects = []
    for f in sources:
        o = filenamegenerator.__next__()
        compiler.do_compile(f, o, defines, includes)
        objects.append(o)

    res = filenamegenerator.__next__()
    rc_compiler.do_compile(rc_file, res)
    objects.append(res)

    #link object files into executable binary
    compiler.do_link(objects, output, library_directories, libraries)

    #delete object files
    for o in objects:
        os.remove(o)


def create_bundle(
        filenamegenerator,
        compiler, rc_compiler,
        sources,
        rc_file,
        defines,
        include_directories,
        library_directories,
        libraries,
        dlls):
    executable = filenamegenerator.__next__()
    build_binary(
        filenamegenerator,
        compiler, rc_compiler,
        sources, rc_file, executable,
        defines, include_directories,
        library_directories, libraries)
    executable += ".exe"
    if os.path.exists("build/HourglassII"):
        shutil.rmtree("build/HourglassII")
    os.mkdir("build/HourglassII")
    shutil.copy(executable, "build/HourglassII/HourglassII.exe")
    for dll in dlls:
        shutil.copy(dll, "build/HourglassII/")
    shutil.copy("level.lua", "build/HourglassII/level.lua")
    os.remove(executable)

boost_include = "C:/Boost/include/boost-1_48/"
sfml_inlcude = "F:/Programming/libraries/SFML-1.6-sdk-windows-mingw/SFML-1.6/include/"
tbb_include = "F:/Programming/libraries/tbb40_233oss/include/"

boost_library_directory = "C:/Boost/lib/"
sfml_library_directory = "F:/Programming/libraries/SFML-1.6-sdk-windows-mingw/SFML-1.6/lib/"
tbb_library_directory = "F:/Programming/libraries/tbb40_233oss/build/windows_ia32_gcc_mingw_release/"

boost_serialization_lib = "boost_serialization-mgw45-mt-1_48"
boost_filesystem_lib = "boost_filesystem-mgw45-mt-1_48"
boost_system_lib = "boost_system-mgw45-mt-1_48"
sfml_system_lib = "sfml-system-s"
sfml_window_lib = "sfml-window-s"
sfml_graphics_lib = "sfml-graphics-s"
tbb_lib = "tbb"
tbb_malloc_lib = "tbbmalloc"

tbb_dll = "F:/Programming/libraries/tbb40_233oss/build/windows_ia32_gcc_mingw_release/tbb.dll"
tbb_malloc_dll = "F:/Programming/libraries/tbb40_233oss/build/windows_ia32_gcc_mingw_release/tbbmalloc.dll"
libgcc_dll = "C:/MinGW/bin/libgcc_s_dw2-1.dll"
libstd_cxx_dll = "C:/MinGW/bin/libstdc++-6.dll"

compiler = basic_gxx_compiler("C:/MinGW/bin/g++")
rc_compiler = windres("C:/MinGW/bin/windres.exe")
seven_zip_binary = "C:/Program Files/7-Zip/7z.exe"

defines = ["BOOST_MULTI_ARRAY_NO_GENERATORS", "LUA_ANSI"]

includes = [boost_include, sfml_inlcude, tbb_include]

library_directories = [
     boost_library_directory, sfml_library_directory,
     tbb_library_directory]

libraries = [
    tbb_lib, tbb_malloc_lib,
    sfml_graphics_lib, sfml_window_lib, sfml_system_lib,
    boost_filesystem_lib, boost_system_lib, boost_serialization_lib]

dlls = [tbb_dll, tbb_malloc_dll, libgcc_dll, libstd_cxx_dll]

rc_file = "windows/resource.rc"
def main():
    files_to_compile = glob.glob("*.cpp") + glob.glob("lua/*.cpp")
    def iota():
        num = 0
        while True:
            yield num
            num += 1

    filenamegenerator = ("build/intermediate/" + str(num) for num in iota())

    if os.path.exists("build"):
        shutil.rmtree("build")
    os.mkdir("build")
    os.mkdir("build/intermediate")

    create_bundle(
        filenamegenerator,
        compiler, rc_compiler,
        files_to_compile,
        rc_file,
        defines,
        includes,
        library_directories,
        libraries,
        dlls)

    shutil.rmtree("build/intermediate")

    #build release package
    os.chdir("build")
    subprocess.call([seven_zip_binary, "a", "HourglassII.7z", "HourglassII/", "-mx9"])
    os.chdir("..")
if __name__ == "__main__":
    main()
