project('simplecube', 'cpp')

m_dep = dependency('m', required : false)
egl_dep = dependency('egl')
glesv2_dep = dependency('glesv2')

simplecube_sources = ['simplecube.cpp',
                      'cube-gles2.cpp',
                      'esTransform.cpp']
executable('simplecube',
           simplecube_sources,
           dependencies : [m_dep, egl_dep, glesv2_dep])
