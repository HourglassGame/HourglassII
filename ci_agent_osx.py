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

#Attempts to update to specified repository revision
#(or to the HEAD revision, if None is specified)
#Returns the actual revision that was updated to.
def svn_up(revision=None):
    output = subprocess.check_output(
        ["svn"] + ["up"]
        + ((["-r"] + [str(revision)]) if revision else []))
    #Sad hack to get latest revision number.
    #Should fix.
    return int(re.search(r"At revision ([0-9]*)\.", str(output)).group(1))

current_revision = svn_up()
while True:
    try:
        revision = svn_up(current_revision)
        if revision > current_revision:
            current_revision = revision
            print("Building revision ", current_revision, " ...")
            build()
            print("Finished building, uploading...")
            upload(current_revision)
            shutil.rmtree("build")
            print("Finished uploading revision ", current_revision)
        else:
            print("Going to sleep")
            time.sleep(60)
    except (KeyboardInterrupt, SystemExit):
        raise
    except:
        print("Unexpected error:", sys.exc_info()[0])
        pass

