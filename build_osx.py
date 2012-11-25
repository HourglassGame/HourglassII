#classes and functions for a few things, just to keep everything organised.
import subprocess
import glob
import shutil
import os
class basic_gxx_compiler:
    def __init__(self, cxxbinary, ccbinary):
        self.cxx = cxxbinary
        self.cc = ccbinary
    def do_compile(self, source, output, defines, include_directories):
        try:
            subprocess.check_output(
                ["/Developer-old/usr/bin/gcc-4.2"]
                +["-x", "c++"]
                +["-arch", self.arch]
                +["-fmessage-length=0"]
                +["-pipe"]
                +["-Wno-trigraphs"]
                +["-fpascal-strings"]
                +["-O3"]
                +["-mdynamic-no-pic"]
                +["-Wmissing-field-initializers"]
                +["-Wmissing-prototypes"]
                +["-Wreturn-type"]
                +["-Woverloaded-virtual"]
                +["-Wmissing-braces"]
                +["-Wparentheses"]
                +["-Wswitch"]
                +["-Wunused-function"]
                +["-Wunused-label"]
                +["-Wunused-parameter"]
                +["-Wunused-variable"]
                +["-Wunused-value"]
                +["-Wuninitialized"]
                +["-Wunknown-pragmas"]
                +["-Wfour-char-constants"]
                +["-Wsign-compare"]
                +["-Wnewline-eof"]
                +["-Wshorten-64-to-32"]
                +["-DBOOST_MULTI_ARRAY_NO_GENERATORS"]
                +["-isysroot", "/Developer-old/SDKs/MacOSX10.6.sdk"]
                +["-mmacosx-version-min=10.4"]
                +["-gdwarf-2"]
                #-iquote /Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/HourglassII-generated-files.hmap
                #-I/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/HourglassII-own-target-headers.hmap
                #-I/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/HourglassII-all-target-headers.hmap
                #-iquote /Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/HourglassII-project-headers.hmap
                +["-Wstrict-overflow"]
                #-F/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/Profile
                #-I/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/Profile/include
                +["-Iext/tbb/include"]
                +["-Iext/SFML/include"]
                +["-Iext/boost/include"]
                #-I/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/DerivedSources/i386
                #-I/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/DerivedSources
                #-c /Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/../../HourglassIISF/trunk/WorldState.cpp
                #-o /Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/Objects-normal/i386/WorldState.o
                + ["-c", source]
                + ["-o", output],
                stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print(e.output.decode("UTF-8"))
            raise
    def do_link(self, sources, output, library_directories, libraries, additional_linker_flags):
        try:
            subprocess.check_output(
                ["/Developer-old/usr/bin/gcc-4.2"]
                +["-arch", self.arch]
                +["-isysroot", "/Developer-old/SDKs/MacOSX10.6.sdk"] 
                #+["-L/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/Profile"]
                +["-Lext/boost/lib"]
                +["-Lext/SFML/lib"]
                +["-Lext/tbb/lib"]
                +["-Lext/freetype/lib"]
                + sources
                #+["-F/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/Profile"]
                #+["-filelist", "/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/Objects-normal/x86_64/HourglassII.LinkFileList"]
                +["-mmacosx-version-min=10.4"]
                +["-lfreetype"]
                +["-lsfml-graphics-s"]
                +["-lsfml-window-s"]
                +["-lsfml-system-s"]
                +["-framework", "CoreFoundation"]
                +["-framework", "Cocoa"]
                +["-framework", "OpenGL"]
                +["-lboost_filesystem"]
                +["-lboost_serialization"]
                +["-lboost_system"]
                +["-lboost_thread"]
                +["-ltbb"]
                +["-ltbbmalloc"]
                +["-shared-libgcc"]
                +["-lstdc++-static"]
                +["-prebind"]
                #+["-o", "/Users/evan/Documents/workspace/HourglassMac/HourglassSetupTest/build/HourglassSetupTest.build/Profile/HourglassIIXcode.build/Objects-normal/x86_64/HourglassII"]
                + ["-o"] + [output],
            #    [self.cc]
             #   + self.additional_flags
              #  + ["-isysroot /Developer-old/SDKs/MacOSX10.6.sdk"]
              #  + list(map(lambda L: "-L" + L, library_directories))
              #  + ["-mmacosx-version-min=10.4"]
              #  + list(map(lambda l: "-l" + l, libraries))
              #  + additional_linker_flags
              #  + ["-shared-libgcc"]
              #  + ["-lstdc++-static"]
              #  + ["-prebind"]
              #  + ["-o"] + [output]
              #  + sources,
                stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print(e.output.decode("UTF-8"))
            raise

class osx_gxx_compiler:
    def __init__(self, cxxbinary, ccbinary, target_arch):
        self.basic_compiler = basic_gxx_compiler(cxxbinary, ccbinary)
        self.basic_compiler.arch = target_arch
    def do_compile(self, source, output, defines, include_directories):
        self.basic_compiler.do_compile(source, output, defines, include_directories)
    def do_link(self, sources, output, library_directories, libraries, additional_linker_flags):
        self.basic_compiler.do_link(sources, output, library_directories, libraries, additional_linker_flags)

class osx_compiler_set:
    def __init__(self, cxxbinary, ccbinary):
        self.x86_64 = osx_gxx_compiler(cxxbinary, ccbinary, "x86_64")
        self.i386 = osx_gxx_compiler(cxxbinary, ccbinary, "i386")

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

def build_universal_binary(
        filenamegenerator,
        compiler_set,
        archs,
        sources, output,
        defines, include_directories,
        library_directories, libraries,
        additional_linker_flags):
    thin_binaries = []
    for arch in archs:
        assert getattr(compilers, arch) != None, "No known way of compiling for architecture: " + arch
        thin_binary = filenamegenerator.__next__()
        build_thin_binary(
            filenamegenerator,
            getattr(compilers, arch),
            sources,
            thin_binary,
            defines,
            includes,
            library_directories,
            libraries,
            additional_linker_flags)
        thin_binaries.append(thin_binary)

    #lipo resulting files
    subprocess.call(
        ["lipo"]
        + thin_binaries
        + ["-create"]
        + ["-output"] + [output])

    #delete thin binaries
    for b in thin_binaries:
        os.remove(b)


def create_bundle(
        filenamegenerator,
        compilers,
        archs,
        files_to_compile,
        defines,
        includes,
        library_directories,
        libraries,
        dlls):
    universal_exe = filenamegenerator.__next__()
    build_universal_binary(
        filenamegenerator,
        compilers,
        archs,
        files_to_compile, universal_exe,
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
    shutil.copy(universal_exe, "build/HourglassII.app/Contents/MacOS/HourglassII")
    for dll in dlls:
        shutil.copy(dll, "build/HourglassII.app/Contents/Frameworks/")
    shutil.copy("src/osx/Info.plist", "build/HourglassII.app/Contents/Info.plist")
    shutil.copy("src/osx/PkgInfo", "build/HourglassII.app/Contents/PkgInfo")
    shutil.copy("basicTriggerSystem.lua", "build/HourglassII.app/Contents/Resources/basicTriggerSystem.lua")
    shutil.copytree("level.lvl", "build/HourglassII.app/Contents/Resources/level.lvl")
    shutil.copytree("levels", "build/HourglassII.app/Contents/Resources/levels")
    shutil.copy("src/osx/HourglassSwirl.icns", "build/HourglassII.app/Contents/Resources/HourglassSwirl.icns")
    #delete lipo'd exe
    os.remove(universal_exe)

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

tbb_dll = "ext/tbb/lib/libtbb.dylib"
tbb_malloc_dll = "ext/tbb/lib/libtbbmalloc.dylib"

freetype_library_directory = "/Users/evan/Programming/freetype/library/lib/"
freetype_lib = "freetype"

compilers = osx_compiler_set("/Developer-old/usr/bin/g++-4.2", "/Developer-old/usr/bin/gcc-4.2")



#Read the config into the internal variables:
archs = ["x86_64", "i386"]

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
    files_to_compile = glob.glob("src/*.cpp") + glob.glob("src/lua/*.cpp")
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
        compilers,
        archs,
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
