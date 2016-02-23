{
  'targets': [
    {
      'target_name': 'EventLog',
      'conditions': [
          ['OS=="win"', {
            'msvs_settings': {
              'VCCLCompilerTool': {                  
                'RuntimeTypeInfo':'true',
                'RuntimeLibrary':'MultiThreadedDLL'
              }
            },
            'msbuild_settings': {
              'ClCompile': {
	        'CompileAs': 'CompileAsCpp',
                'ExceptionHandling': 'Async',
                'CompileAsManaged':'true'
              }
            }
          }]
      ],
      'include_dirs': [
          '.',
          "<!(node -e \"require('nan')\")"
      ],
      'sources': [
        'src/EventLog.cpp'
      ]
    }
  ]
}
