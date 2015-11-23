# Herbie - Diagnostics
Herbie is a small CPU/Memory Monitoring Widget for Windows. It is licensed under the GNU General Public License v3.

## Build Herbie (Windows)
### Compile dependencies
- `Qt` SDK or the compiled sources (>= 5.4.0)

### Compile Herbie 
1. Open CMake GUI
2. set your Herbie/src folder to `where is the source code`
3. choose a build folder (e.g. build2015-x64)
4. Set `QT_QMAKE_EXECUTABLE` by locating the qmake.exe
5. Hit `Configure`then `Generate`
6. Open the `Herbie.sln` which is in your new build directory
7. Right-click the DkHerbie project and choose `Set as StartUp Project`
8. Compile the Solution

### If anything did not work
- check if your Qt is set correctly (otherwise set the path to `qt_install_dir/qtbase/bin/qmake.exe`)
- check if your builds proceeded correctly