import sys
import time
import re
import subprocess
import shutil
import traceback
import os.path
import os
import imp

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

def build():
    import build_windows
    imp.reload(build_windows)
    build_windows.main()
    pass

def upload(revision):
    release_filename = "build/HourglassIIr"+str(revision)+".7z"
    #rename file
    shutil.copy(
        "build/HourglassII.7z",
         release_filename)
    #scp file
    runsubprocess(
        ["F:/Program Files/PuTTY/pscp.exe"]
        + ["-i", "Sourceforge.ppk"]
        + [release_filename]
        + ["evanwallace,hourglassii@frs.sourceforge.net"
           +":/home/frs/project/h/ho/hourglassii/Release/Windows/"])

def svn_up():
    print("Updating...")
    output = runsubprocess(["svn"] + ["up"]).decode("UTF-8").replace("\r\n", "\n")
    print("Updated to revision",
      re.search(r"^((At revision )|(Updated to revision ))([0-9]*)\.$", output, re.MULTILINE).group(4))

def svnversion():
    return int(
      re.search(r"Last Changed Rev: ([0-9]*)", runsubprocess(["svn", "info"]).decode("UTF-8")).group(1))

def main():
    try:
        current_revision = svnversion()
        svn_up()
        revision = svnversion()
        if revision > current_revision:
            print("Building revision", revision, "...")
            build()
            print("Finished building, uploading...")
            upload(revision)
            shutil.rmtree("build")
            print("Finished uploading.")
        else:
            print("Going to sleep.")
            time.sleep(60)
    except (KeyboardInterrupt, SystemExit):
        raise
    except:
        print("Unexpected error:")
        traceback.print_exc()
        time.sleep(1)

if __name__ == "__main__": main()
