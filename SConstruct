import os

USE_TOOL = 'msvc'
# mingw support is not working yet.
#USE_TOOL = 'mingw'

if USE_TOOL == 'msvc':
	env = Environment(
		ENV = os.environ,
		CCFLAGS = '-Os -Og -Oy -Zl -Gy -DWIN32 -D_WINDOWS -DSTRICT -DNDEBUG',
		CPPPATH = [r'p:\code\wtl70\include',r'p:\external\crypto521'],
		LIBPATH = [r'p:\lib'],
			LIBS = ['cryptlib','gdi32','comdlg32']
	)
	
	res = env.RES('locknote.rc', RCFLAGS= '-DNDEBUG -l1033 -v' )
	env.Program('LockNote',['locknote.cpp','stdafx.cpp',res])

elif USE_TOOL == 'mingw':	
	rootpath = 'c:\mingw'
	os.environ['PATH'] += ';' + os.path.join(rootpath, 'bin')
	
	includepath = [os.path.join(rootpath,'include')]	
	includepath += [r'C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\atlmfc\include']

	env = Environment(
		ENV = os.environ,
		tools = ['mingw'],
		CCFLAGS = '-DWIN32 -D_WINDOWS -DSTRICT -DNDEBUG',
		CPPPATH = includepath + [r'p:\code\wtl70\include',r'p:\external\crypto521'],
		LIBPATH = [r'p:\lib'],
			LIBS = ['cryptlib','gdi32','comdlg32']
	)
	
	#res = env.RES('locknote.rc', RCFLAGS= '-DNDEBUG -l1033 -v' )
	env.Program('LockNote',['locknote.cpp','stdafx.cpp'])
