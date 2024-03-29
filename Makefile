CXXFLAGS := -std=c++17 -g

all: main

main: main.o
	$(CXX) $^ -lvulkan -lglfw

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

main.exe: main.o
	lld-link.exe -out:main.exe -nologo main.o -libpath:"c:/VulkanSDK/1.2.198.1/Lib/" -defaultlib:kernel32.lib -defaultlib:msvcrt.lib -defaultlib:kernel32.lib -defaultlib:user32.lib -defaultlib:vulkan-1.lib

#"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\VC\\Tools\\Llvm\\x64\\bin\\clang-cl.exe" -cc1 -triple x86_64-pc-windows-msvc19.29.30133 -emit-obj -mrelax-all -mincremental-linker-compatible --mrelax-relocations -disable-free -disable-llvm-verifier -discard-value-names -main-file-name main.cpp -mrelocation-model pic -pic-level 2 -mframe-pointer=none -relaxed-aliasing -fmath-errno -fno-rounding-math -mconstructor-aliases -munwind-tables -target-cpu x86-64 -mllvm -x86-asm-syntax=intel -tune-cpu generic -D_MT -flto-visibility-public-std --dependent-lib=libcmt --dependent-lib=oldnames -stack-protector 2 -fms-volatile -fdiagnostics-format msvc -v -resource-dir "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\VC\\Tools\\Llvm\\x64\\lib\\clang\\12.0.0" -D UNICODE=1 -D _UNICODE=1 -internal-isystem "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\VC\\Tools\\Llvm\\x64\\lib\\clang\\12.0.0\\include" -internal-isystem "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.29.30133/include" -internal-isystem "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/include/um" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/ucrt" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/shared" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/um" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/winrt" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/cppwinrt" -internal-isystem "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.29.30133/include" -internal-isystem "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/include/um" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/ucrt" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/shared" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/um" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/winrt" -internal-isystem "C:/Program Files (x86)/Windows Kits/10/include/10.0.19041.0/cppwinrt" -fdeprecated-macro -fdebug-compilation-dir "c:\\Users\\chris\\Documents\\projects\\vulkan-tutorial" -ferror-limit 19 -fno-use-cxa-atexit -fms-extensions -fms-compatibility -fms-compatibility-version=19.29.30133 -std=c++14 -fdelayed-template-parsing -faddrsig -o "C:\\msys64\\tmp\\main-793539.obj" -x c++ main.cpp
#"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\VC\\Tools\\Llvm\\x64\\bin\\lld-link" -out:main.exe "-libpath:C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\BuildTools\\VC\\Tools\\Llvm\\x64\\lib\\clang\\12.0.0\\lib\\windows" -nologo "C:\\msys64\\tmp\\main-793539.obj"

#C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.29.30133/include
#C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/Llvm/x64/lib/clang/12.0.0/lib/windows
