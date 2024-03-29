PATH_TO_EMSDK=~/emsdk/
source $PATH_TO_EMSDK/emsdk_env.sh

projectdir=$(dirname $PWD)
echo $projectdir

emcc -o PhysicsBox.html $projectdir/main.c -Wall -std=c11 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os -I. -I ~/raylib/src -I ~/raylib/src/external -L. -L ~/raylib/src -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 --shell-file ~/raylib/src/shell.html ~/raylib/src/web/libraylib.a -DPLATFORM_WEB -s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' -s EXPORTED_RUNTIME_METHODS=ccall -sGL_ENABLE_GET_PROC_ADDRESS
