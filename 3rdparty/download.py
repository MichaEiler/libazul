import os
import argparse
from zipfile import ZipFile
import platform
import urllib.request

def execute_command(command):
    ret = os.system(command)
    if ret != 0:
        raise RuntimeError("The following command failed with exit code: {0}\n\t{1}".format(ret, command))

def move_to_script_directory():
    dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(dir)

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

if __name__ == "__main__":
    move_to_script_directory()

    build_dependencies = [ GitRepository("https://github.com/google/googletest.git", "googletest", "master") ]

    for dependency in build_dependencies:
        dependency.resolve()

    print("Preparing dependencies completed!")
