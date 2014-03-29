# copyDLLs.py
# Copy the necessary Windows DLLs to the executable directory.
# usage: cd tools/; python copyDLLs.py
# Or just double-click copyDLLs.py .

import os
import shutil

# These directories depend on local setup, where repositories are checked out.
# @todo Maybe grab this directory from CMakeCache
dllHome = "C:/lib/"
projectHome = "../"

# List of DLLs to copy. Paths are listed in components to easily accomodate
# alternate configurations.
# @todo Use python's path handling library for correctness
dllList = [
	["AntTweakBar_116/" , "AntTweakBar/lib/"    , "AntTweakBar.dll"],
	["glew/"            , "bin/"                , "glew32.dll"     ],
]

debugDllList = [
]

releaseDllList = [
]

def assembleBuild(buildname, dllHome, dllList):
	""" Copy a list of dlls into a build directory.
	"""
	print "__Assembling: " + buildname
	dllDest = projectHome + "build/" + buildname + "/"
	for f in dllList:
		src = dllHome + f[0] + f[1] + f[2]
		dst = dllDest + f[2]
		print "  copy\n    ",src,"\n    ",dst
		shutil.copyfile(src, dst)

assembleBuild("Debug", dllHome, dllList+debugDllList)
assembleBuild("Release", dllHome, dllList+releaseDllList)
