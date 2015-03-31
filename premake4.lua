--
-- Premake4 build script (http://industriousone.com/premake/download)
--

solution 'calmdump'
    language 'C++'    
    targetdir 'bin'
    configurations {'Debug', 'Release'}
    platforms {'x32','x64'}

    configuration "Debug"
        flags { 'Symbols' }

    configuration "Release"
        flags { 'Symbols', 'Optimize' }

    project 'calmdump'
        kind 'StaticLib'
        location 'build'
        flags
        {
            'ExtraWarnings',
        }
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0502',
            'NOMINMAX',
        }
        files
        {
            'src/*.h',
            'src/*.cpp',
        }

    project 'test'
        kind 'ConsoleApp'
        location 'build'
        files
        {
            'tests/*.h',
            'tests/*.cpp',
        }
        includedirs 
        {
            'src'
        }
        links 
        {
            'calmdump'
        }
