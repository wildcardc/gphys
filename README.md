-----------------------------------------------------------------------------
---- Visual Studio 2012 Template Project for C++/DirectX11 Applications -----
---- Marc Treib, Florian Ferstl, Sept 2013 ----------------------------------
-----------------------------------------------------------------------------

This solution contains the following components:

 - AntTweakBar 1.16 
   "AntTweakBar is a small and easy-to-use C/C++ library that allows programmers to quickly add a light and 
   intuitive graphical user interface into graphic applications based on OpenGL (compatibility and core 
   profiles), DirectX 9, DirectX 10 or DirectX 11 to interactively tweak parameters on-screen."
   Documentation: http://anttweakbar.sourceforge.net

 - DirectX Tool Kit, July 2013 (the 'MakeSpriteFont' project is part of the DirectXTK)
   "The DirectX Tool Kit (aka DirectXTK) is a collection of helper classes for writing Direct3D 11 code in C++."
   Documentation: http://directxtk.codeplex.com/
 
 - DXUT, September 2013 (11.03) (formerly part of the DirectX SDK's)
   Updated version for VS2012 and Windows 8.0 SDK. Modified as suggested for use in combination with DirectXTK.
   (see http://blogs.msdn.com/b/chuckw/archive/2013/09/14/dxut-for-win32-desktop-update.aspx)

 - Effects 11 (FX11), October 24, 2012 (11.03) (formerly part of the DirectX SDK's)
   Updated version for VS2012 and Windows 8.0 SDK.
   (see http://blogs.msdn.com/b/chuckw/archive/2012/10/24/effects-for-direct3d-11-update.aspx)

 - Main Project:
   > "main.cpp": Structured like a typical DXUT-based application. It contains example code that correctly
   integrates and demonstrates how to use AntTweakBar (for a simple GUI) and DirectXTK (for simple, 
   "fixed-function" rendering of common primitives).
   > "effect.fx": Starting point for custom shaders, already loaded in main.cpp. If  you don't need custom 
   shaders, you can safely ignore or even delete it (and any code related to "g_pEffect" in main.cpp)


Further Note:

 - DirectXMath (all functions/types starting with XM*): DirectXTK and the new DXUT are both based on the 
   new DirectXMath API for linear algebra that comes with the Windows 8.0 SDK (replacing the old D3DXMath). 
   You can use it for all of your linear algebra tasks.
   Documentation: http://msdn.microsoft.com/en-us/library/windows/desktop/hh437833%28v=vs.85%29.aspx