project('Gflit', 'cpp')

# Find dependencies
gtk_dep = dependency('gtk4')
curl_dep = dependency('libcurl')

# Define the executable
executable('hello-world-gtk',
           'src/main.c++',
           dependencies: [gtk_dep, curl_dep],
           install: true)
