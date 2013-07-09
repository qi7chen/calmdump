
solution 'calmdump'
    configurations {'Debug', 'Release'}
    location ('./' .. (_ACTION or '') )
    language 'C++'
    flags {
        'No64BitChecks',
        'ExtraWarnings',
    }
    configuration "Debug"
        defines     '_DEBUG'
        flags       { 'Symbols' }
			
    configuration "Release"
        defines     'NDEBUG'
        flags       { 'Symbols', 'Optimize' }
        
    project 'libcalmdump'
        kind 'StaticLib'
        
        defines {
            'WIN32_LEAN_AND_MEAN',
        }
        
        files {
            '../src/**.h',
            '../src/**.cpp',
        }
        
    project 'tests'
        kind 'ConsoleApp'
        
        files {
            '../tests/**.h',
            '../tests/**.cpp',
        }
        
        links ( 'libcalmdump' )
        