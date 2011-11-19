import build_windows
import sys
import time
import re
import subprocess
import shutil
import traceback
import os.path
import os
def build():
    build_windows.main()
    pass

def upload(revision):
    release_filename = "build/HourglassIIr"+str(revision)+".7z"
    #rename file
    shutil.copy(
        "build/HourglassII.7z",
         release_filename)
    #scp file
    subprocess.call(
        ["F:/Program Files/PuTTY/pscp.exe"]
        + ["-i", "F:/User~Docs/Keven/Workspace/ssh_key/Sourceforge.ppk"]
        + [release_filename]
        + ["evanwallace,hourglassii@frs.sourceforge.net"
           +":/home/frs/project/h/ho/hourglassii/Release/Windows/"])

def svn_up():
    print("Updating...")
    output = subprocess.check_output(["svn"] + ["up"]).decode("UTF-8").replace("\r\n", "\n")
    print("Updated to revision",
      re.search(r"^((At revision )|(Updated to revision ))([0-9]*)\.$", output, re.MULTILINE).group(4))

def svnversion():
    return int(
      re.search(r"Last Changed Rev: ([0-9]*)", subprocess.check_output(["svn", "info"]).decode("UTF-8")).group(1))

current_revision = svnversion()
while True:
    try:
        if os.path.exists("build/"):
            shutil.rmtree("build/")
        svn_up()
        revision = svnversion()
        if revision > current_revision:
            current_revision = revision
            print("Building revision", current_revision, "...")
            build()
            print("Finished building, uploading...")
            upload(current_revision)
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
        pass
