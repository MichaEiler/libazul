import os
import argparse
from zipfile import ZipFile
import platform
import urllib.request
import argparse

def execute_command(command):
    ret = os.system(command)
    if ret != 0:
        raise RuntimeError("The following command failed with exit code: {0}\n\t{1}".format(ret, command))

def move_to_script_directory():
    dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(dir)

# https://stackoverflow.com/questions/42326428/zipfile-in-python-file-permission
ZIP_UNIX_SYSTEM = 3
def extract_all_with_permission(zf, target_dir):
    for info in zf.infolist():
        extracted_path = zf.extract(info, target_dir)

        if info.create_system == ZIP_UNIX_SYSTEM:
            unix_attributes = info.external_attr >> 16
            if unix_attributes:
                os.chmod(extracted_path, unix_attributes)

class GitRepository:
    def __init__(self, url, name, revision):
        self.__url = url
        self.__name = name
        self.__revision = revision
    
    def __clone(self):
        print("Cloning {0}".format(self.__url))
        execute_command("git clone {0}".format(self.__url))

    def __checkout(self, revision):
        print("Checking out revision {0}".format(self.__url))
        execute_command("cd {0} && git checkout {1}".format(self.__name, self.__revision))

    def resolve(self):
        if not os.path.exists("./{0}".format(self.__name)):
            self.__clone()
            self.__checkout(self.__revision)

class AndroidNdk:
    def __init__(self, version):
        self.__version = version
        self.__archive_name = "android-ndk-{0}-{1}-x86_64.zip".format(self.__version, platform.system().lower())
        self.__url = "https://dl.google.com/android/repository/{0}".format(self.__archive_name)
    
    def __download(self):
        if os.path.exists("./{0}".format(self.__archive_name)):
            return

        print("Downloading {0}".format(self.__url))
        urllib.request.urlretrieve(self.__url, "./{0}".format(self.__archive_name))

    def __unpack(self):
        if os.path.exists("./android-ndk-{0}".format(self.__version)):
            return

        print("Unpacking {0}".format(self.__archive_name))
        with ZipFile("./{0}".format(self.__archive_name), "r") as archive:
            extract_all_with_permission(archive, "./")

    def resolve(self):
        self.__download()
        self.__unpack()

if __name__ == "__main__":
    move_to_script_directory()

    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--mobile", help="Also download iOS/Android SDKs", action="store_true", default=False)
    args = parser.parse_args()


    build_dependencies = [ GitRepository("https://github.com/google/googletest.git", "googletest", "master")]

    if args.mobile == True:
        build_dependencies.append(AndroidNdk("r19c"))

        if platform.system() == "Darwin":
            build_dependencies.append(GitRepository("https://github.com/leetal/ios-cmake.git", "ios-cmake", "8ffdeb0"))
    
    for dependency in build_dependencies:
        dependency.resolve()

    print("Preparing dependencies completed!")
