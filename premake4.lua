--
-- Premake4 build script (http://industriousone.com/premake/download)
--

solution 'calmdump'
    language 'C++'    
    targetdir 'bin'
    configurations {'Debug', 'Release'}

    configuration "Debug"
        flags { 'Symbols' }

    configuration "Release"
        flags { 'Symbols', 'Optimize' }

    project 'libcalmdump'
        kind 'StaticLib'
        location 'build'
        uuid '90671AD7-3805-47bb-A425-D7F5F8C798DA'
        flags
        {
            'ExtraWarnings',
        }
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0502',
        }

        files
        {
            'src/**.h',
            'src/**.cpp',
        }

    project 'tests'
        kind 'ConsoleApp'
        location 'build'
        uuid '4AB5B35B-CD36-40fa-94C3-A4276D120394'
        files
        {
            'tests/**.h',
            'tests/**.cpp',
        }

        links ( 'libcalmdump' )
