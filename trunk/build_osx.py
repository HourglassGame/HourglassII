#classes and functions for a few things, just to keep everything organised.
import subprocess
import glob
import shutil
import os
class basic_gxx11_compiler:
    def __init__(self, cxxbinary, ccbinary):
        self.cxx = cxxbinary
        self.cc = ccbinary
    def do_compile(self, source, output, defines, include_directories):
        try:
            subprocess.check_output(
                [self.cxx]
                + list(map(lambda d: "-D" + d, defines))
                + list(map(lambda i: "-I" + i, include_directories))
                + ["-O3"]
                + ["-mmacosx-version-min=10.4"]
                + ["-std=gnu++0x"]
                + ["-c"] + [source]
                + ["-o"] + [output], stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print(e.output.decode("UTF-8"))
            raise
    def do_link(self, sources, output, library_directories, libraries, additional_linker_flags):
        try:
            subprocess.check_output(
                [self.cc]
                + list(map(lambda L: "-L" + L, library_directories))
                + list(map(lambda l: "-l" + l, libraries))
                + additional_linker_flags
                + ["-shared-libgcc"]
                + ["-lstdc++-static"]
                + ["-mmacosx-version-min=10.4"]
                + ["-std=gnu++0x"]
                + ["-o"] + [output]
                + sources, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print(e.output.decode("UTF-8"))
            raise

def build_thin_binary(
    filenamegenerator,
    compiler,
    sources, output,
    defines, include_directories,
    library_directories, libraries, additional_linker_flags):
    #compile sources into object files
    objects = []
    for f in sources:
        o = filenamegenerator.__next__()
        compiler.do_compile(f, o, defines, includes)
        objects.append(o)

    #link object files into executable binary
    compiler.do_link(objects, output, library_directories, libraries, additional_linker_flags)

    #delete object files
    for o in objects:
        os.remove(o)


def create_bundle(
    filenamegenerator,
    compilers,
    files_to_compile,
    defines,
    includes,
    library_directories,
    libraries,
    dlls):
    executable_binary = filenamegenerator.__next__()
    build_thin_binary(
        filenamegenerator,
        compilers,
        files_to_compile, executable_binary,
        defines, includes,
        library_directories, libraries,
        ["-framework", "CoreFoundation"]
        +["-framework", "Cocoa"]
        +["-framework", "OpenGL"])

    #build app bundle
    os.mkdir("build/HourglassII.app")
    os.mkdir("build/HourglassII.app/Contents")
    os.mkdir("build/HourglassII.app/Contents/MacOS")
    os.mkdir("build/HourglassII.app/Contents/Resources")
    os.mkdir("build/HourglassII.app/Contents/Frameworks")
    shutil.copy(executable_binary, "build/HourglassII.app/Contents/MacOS/HourglassII")
    for dll in dlls:
        shutil.copy(dll, "build/HourglassII.app/Contents/Frameworks/")
    shutil.copy("osx/Info.plist", "build/HourglassII.app/Contents/Info.plist")
    shutil.copy("osx/PkgInfo", "build/HourglassII.app/Contents/PkgInfo")
    shutil.copy("level.lua", "build/HourglassII.app/Contents/Resources/level.lua")
    shutil.copy("osx/HourglassSwirl.icns", "build/HourglassII.app/Contents/Resources/HourglassSwirl.icns")

    #delete executable
    os.remove(executable_binary)

def create_redistributable():
    subprocess.call(["hdiutil", "create", "-srcfolder", "build/HourglassII.app", "build/HourglassII.dmg"])

#configuration: library locations, some configuration of toolsets
#the configuration is here for now in order to get it done
#ideally it would be in a separate file
boost_include = "/Users/evan/Programming/boost/library/include/"
boost_library_directory = "/Users/evan/Programming/boost/library/lib/"
boost_serialization_lib = "boost_serialization"
boost_filesystem_lib = "boost_filesystem"
boost_system_lib = "boost_system"
boost_thread_lib = "boost_thread"

sfml_inlcude = "/Users/evan/Programming/SFML/library/include/"
sfml_library_directory = "/Users/evan/Programming/SFML/library/lib/"
sfml_system_lib = "sfml-system-s"
sfml_window_lib = "sfml-window-s"
sfml_graphics_lib = "sfml-graphics-s"

tbb_include = "/Users/evan/Programming/tbb/library/include/"
tbb_library_directory = "/Users/evan/Programming/tbb/library/lib/"
tbb_lib = "tbb"
tbb_malloc_lib = "tbbmalloc"

tbb_dll = "/Users/evan/Programming/tbb/library/lib/libtbb.dylib"
tbb_malloc_dll = "/Users/evan/Programming/tbb/library/lib/libtbbmalloc.dylib"

freetype_library_directory = "/Users/evan/Programming/freetype/library/lib/"
freetype_lib = "freetype"

compiler = basic_gxx11_compiler("/opt/local/bin/g++-mp-4.6", "/opt/local/bin/gcc-mp-4.6")

#Read the config into the internal variables:
defines = ["BOOST_MULTI_ARRAY_NO_GENERATORS", "LUA_ANSI"]

includes = [boost_include, sfml_inlcude, tbb_include]

library_directories = [
     boost_library_directory, sfml_library_directory,
     tbb_library_directory, freetype_library_directory]

libraries = [
     boost_serialization_lib, boost_filesystem_lib, boost_system_lib, boost_thread_lib,
     sfml_system_lib, sfml_window_lib, sfml_graphics_lib,
     tbb_lib, tbb_malloc_lib,
     freetype_lib]

dlls = [tbb_dll, tbb_malloc_dll]

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

    #build bundle
    create_bundle(
        filenamegenerator,
        compiler,
        files_to_compile,
        defines,
        includes,
        library_directories,
        libraries,
        dlls)

    #build release package
    create_redistributable()

    shutil.rmtree("build/intermediate")

if __name__ == "__main__":
    main()
