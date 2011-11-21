import subprocess
import shutil
import re
import sys
import time
import imp
def build():
    import build_osx
    imp.reload(build_osx)
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
      re.search(r"^((At revision )|(Updated to revision ))([0-9]*)\.$", output.decode("UTF-8"), re.MULTILINE).group(4))

#returns the current revision of the working copy
def svnversion():
    return int(
      re.search(
        r"Last Changed Rev: ([0-9]*)",
        subprocess.check_output(["svn", "info"]).decode("UTF-8")).group(1))

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
        print("Unexpected error:", sys.exc_info())
        time.sleep(1)

if __name__ == "__main__":
    main()