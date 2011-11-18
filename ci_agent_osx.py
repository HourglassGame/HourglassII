import build_osx
import subprocess
import shutil
import re
import sys
import time
def build():
    build_osx.main()
    pass

def upload(revision):
    release_filename = "build/HourglassIIr"+str(revision)+".dmg"
    #rename file
    shutil.copy(
        "build/HourglassII.dmg",
         release_filename)
    #scp file
    subprocess.call(
        ["scp"]
        + ["-B"]
        + ["-i","/Users/evan/.ssh/id_dsa"]
        + [release_filename]
        + ["evanwallace,hourglassii@frs.sourceforge.net"
           +":/home/frs/project/h/ho/hourglassii/Release/OSX/"])

#Attempts to update to HEAD
def svn_up():
    print("Updating...")
    output = subprocess.check_output(["svn"] + ["up"])
    print("Updated to revision",
      re.search(r"At revision ([0-9]*)\.", str(output)).group(1))

#returns the current revision of the working copy
def svnversion():
    return int(
      re.search(
        r"Last Changed Rev: ([0-9]*)",
        str(subprocess.check_output(["svn", "info"]))).group(1))

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
            shutil.rmtree("build")
            print("Finished uploading.")
        else:
            print("Going to sleep.")
            time.sleep(60)
    except (KeyboardInterrupt, SystemExit):
        raise
    except:
        print("Unexpected error:", sys.exc_info())
        pass

