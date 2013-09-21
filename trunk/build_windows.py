import glob
import shutil
import os
import os.path
import subprocess

def runsubprocess(command_and_args):
    try:
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        return subprocess.check_output(
            command_and_args,
            stderr=subprocess.STDOUT,
            startupinfo=startupinfo)
    except subprocess.CalledProcessError as e:
        print(e.output.decode("UTF-8"))
        raise

class basic_gxx_compiler:
    def __init__(self, cxxbinary):
        self.cxx = cxxbinary
    def do_compile(self, source, output, defines, include_directories):
        runsubprocess(
            [self.cxx]
            + list(map(lambda d: "-D" + d, defines))
            + list(map(lambda i: "-I" + i, include_directories))
            + ["-ftemplate-depth=128"]
            + ["-O3"]
            + ["-march=i486"]
            + ["-std=c++0x"]
            + ["-c"] + [source]
            + ["-o"] + [output])
    def do_link(self, sources, output, library_directories, libraries):
        runsubprocess(
            [self.cxx]
            + ["-s"]
            + list(map(lambda L: "-L" + L, library_directories))
            + ["-Wl,-subsystem,windows"]
            + ["-Xlinker", "--enable-auto-import"]
            + ["-o"] + [output]
            + sources
            + list(map(lambda l: "-l" + l, libraries)))

class windres:
    def __init__(self, windresbinary):
        self.windres = windresbinary
    def do_compile(self, input, output):
        runsubprocess(
            [self.windres]
            + [input]
            + ["-o"] + [output])

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
    shutil.copytree("data", "build/HourglassII/data")
    os.remove(executable)

compiler = basic_gxx_compiler("C:/MinGW/bin/g++")
rc_compiler = windres("C:/MinGW/bin/windres.exe")
seven_zip_binary = "C:/Program Files/7-Zip/7z.exe"

defines = ["BOOST_MULTI_ARRAY_NO_GENERATORS", "LUA_ANSI", "BOOST_THREAD_USE_LIB", "TBB_USE_CAPTURED_EXCEPTION", "BOOST_THREAD_USES_MOVE", "NDEBUG"]

includes = ["ext/boost/include/", "ext/SFML/include/","ext/tbb/include/"]

library_directories = ["ext/boost/lib/", "ext/SFML/lib/", "ext/tbb/lib/"]

libraries = [
    "tbb", "tbbmalloc",
    "sfml-graphics-2", "sfml-window-2", "sfml-system-2", "opengl32", "glu32", "gdi32", "winmm",
    "boost_serialization-mgw47-mt-1_54", "boost_filesystem-mgw47-mt-1_54", "boost_system-mgw47-mt-1_54", "boost_thread-mgw47-mt-1_54", "boost_chrono-mgw47-mt-1_54"]

dlls = [
    "ext/tbb/lib/tbb.dll", "ext/tbb/lib/tbbmalloc.dll",
    "C:/MinGW/bin/libgcc_s_dw2-1.dll", "C:/MinGW/bin/libstdc++-6.dll",
    "ext/sfml/lib/sfml-graphics-2.dll", "ext/sfml/lib/sfml-window-2.dll", "ext/sfml/lib/sfml-system-2.dll"]

rc_file = "src/windows/resource.rc"

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
    runsubprocess([seven_zip_binary, "a", "HourglassII.7z", "HourglassII/", "-mx9"])
    os.chdir("..")

if __name__ == "__main__":
    main()
