import build_windows
import sys
import time
import re
import subprocess
import shutil

def build():
    build_windows.main()
    pass

def upload(revision):
    release_filename = "build/HourglassIIr"+str(revision)+".7z"
    print("Release Filename is", release_filename)
    #rename file
    shutil.copy(
        "build/HourglassII.7z",
         release_filename)
    print("Copied release to give it its new name")
    #scp file
    subprocess.call(
        ["F:/Program Files/PuTTY/pscp.exe"]
        + [release_filename]
        + ["evanwallace,hourglassii@frs.sourceforge.net"
           +":/home/frs/project/h/ho/hourglassii/Release/Windows/"])
    print("Uploaded the release")

def svn_up():
    print("Updating...")
    output = subprocess.check_output(["svn"] + ["up"]).decode("UTF-8").replace("\r\n", "\n")
    print(output)
    print("Updated to revision",
      re.search(r"^((At revision )|(Updated to revision ))([0-9]*)\.$", output, re.MULTILINE).group(4))

def svnversion():
    print("Getting current working version...")
    output = subprocess.check_output(["svn", "info"]).decode("UTF-8")
    version = int(
      re.search(r"Last Changed Rev: ([0-9]*)", output).group(1))
    print("Current working version is:", version)
    return version

current_revision = svnversion()
while True:
    try:
        svn_up()
        revision = svnversion()
        if revision > current_revision:
            current_revision = revision
            print("Building revision", current_revision, "...")
            build()
            print("Finished building, uploading...")
            upload(current_revision)
            shutil.rmtree("build/")
            print("Finished uploading.")
        else:
            print("Going to sleep.")
            time.sleep(60)
    except (KeyboardInterrupt, SystemExit):
        raise
    except:
        print("Unexpected error:", sys.exc_info())
        pass
