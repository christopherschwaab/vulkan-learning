{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
    buildInputs = with pkgs.buildPackages; [
        glfw
        vulkan-tools
        vulkan-helper
        vulkan-headers
        vulkan-tools-lunarg
        vulkan-loader
        vulkan-extension-layer
        vulkan-validation-layers
        vulkan-utility-libraries
        llvmPackages_15.libllvm.lib
        zlib
        zstd
        libdrm
        udev
        expat
        xorg.libxcb
        xorg.libX11
        xorg.libxshmfence
        gcc-unwrapped
        wayland
        gdb
        libdecor
    ];

 shellHook = ''
    export PATH="${pkgs.vulkan-validation-layers}/bin:$PATH"
    export LD_LIBRARY_PATH="${pkgs.llvmPackages_15.libllvm.lib}/lib:${pkgs.libdrm.out}/lib:${pkgs.zlib.out}/lib:${pkgs.zstd.out}/lib:${pkgs.udev.out}/lib:${pkgs.expat.out}/lib:${pkgs.xorg.libxcb.out}/lib:${pkgs.xorg.libX11}/lib:${pkgs.xorg.libxshmfence.out}/lib:${pkgs.gcc-unwrapped.lib}/lib:${pkgs.wayland.out}/lib:${pkgs.libdecor.out}/lib:$LD_LIBRARY_PATH"
    export VK_ADD_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d:$VK_ADD_LAYER_PATH
  '';
}
