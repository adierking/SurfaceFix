#define fix_surfaces
// fix_surfaces([dllpath]) - Initializes Surface Fix and fixes the surface functions.
//
// Argument 1 (dllpath, optional) - The path to the DLL file.
//                                  If this is not specified, it is assumed to be "SurfaceFix.dll".

var dllpath;

// Get the path to the DLL file
if (is_real(argument0))
{
    dllpath = "SurfaceFix.dll";
}
else
{
    dllpath = argument0;
}

// Define functions
global.__SFIX_FixSurfaces               = external_define(dllpath, "FixSurfaces", dll_cdecl, ty_real, 0);
global.__SFIX_ClearDepthBuffer          = external_define(dllpath, "ClearDepthBuffer", dll_cdecl, ty_real, 0);
global.__SFIX_GetCurrentSurface         = external_define(dllpath, "GetCurrentSurface", dll_cdecl, ty_real, 0);
global.__SFIX_SurfaceToString           = external_define(dllpath, "SurfaceToString", dll_cdecl, ty_string, 2, ty_real, ty_real);
global.__SFIX_SurfaceFromString         = external_define(dllpath, "SurfaceFromString", dll_cdecl, ty_real, 2, ty_real, ty_string);
global.__SFIX_WriteSurfaceToBinaryFile  = external_define(dllpath, "WriteSurfaceToBinaryFile", dll_cdecl, ty_real, 2, ty_real, ty_real);
global.__SFIX_ReadSurfaceFromBinaryFile = external_define(dllpath, "ReadSurfaceFromBinaryFile", dll_cdecl, ty_real, 2, ty_real, ty_real);
global.__SFIX_ChangeDepthBuffer         = external_define(dllpath, "ChangeDepthBuffer", dll_cdecl, ty_real, 1, ty_real);
global.__SFIX_EnableDepthWriting        = external_define(dllpath, "EnableDepthWriting", dll_cdecl, ty_real, 1, ty_real);

// Define constants
globalvar df_binary, df_text;
df_binary = 0;
df_text = 1;

// Call the initialization function
external_call(global.__SFIX_FixSurfaces);
#define surface_get_target
// surface_get_target() - Returns the ID of the surface that you are currently drawing to.
//                        If you are currently drawing to the screen, -1 is returned.

return external_call(global.__SFIX_GetCurrentSurface);
#define surface_write
// surface_write(id, format) - Converts a surface into a string and returns it.
//                             See the documentation for more information.
// Argument 1 (id) - The ID of the surface.
// Argument 2 (format) - The data format; either df_binary or df_text.

return external_call(global.__SFIX_SurfaceToString, argument0, argument1);
#define surface_read
// surface_read(id, str) - Reads the contents of a string created by surface_write() into a surface.
//                         See the documentation for more information.

return external_call(global.__SFIX_SurfaceFromString, argument0, argument1);
#define file_bin_write_surface
// file_bin_write_surface(file, id) - Writes a surface to a binary file.
//                                    See the documentation for more information.
// Argument 1 (file) - The ID of the binary file to write to.
// Argument 2 (id) - The ID of the surface to write.

return external_call(global.__SFIX_WriteSurfaceToBinaryFile, argument0, argument1);
#define file_bin_read_surface
// file_bin_write_surface(file, id) - Reads a surface from a binary file.
//                                    See the documentation for more information.
// Argument 1 (file) - The ID of the binary file to read from.
// Argument 2 (id) - The ID of the surface to read into.

return external_call(global.__SFIX_ReadSurfaceFromBinaryFile, argument0, argument1);
#define depth_change
// depth_change(id) - Temporarily changes the current depth buffer to that of another surface.
//                    See the documentation for more information.
// Argument 1 (id) - The ID of the surface whose depth buffer should be changed to.
//                   Pass in -1 to switch to the screen's depth buffer.

return external_call(global.__SFIX_ChangeDepthBuffer, argument0);
#define depth_write_enable
// depth_write_enable(enable) - Enables/disables depth writing.
//                              See the documentation for more information.
// Argument 1 (enable) - Pass in true to enable depth writing and false to disable it.

external_call(global.__SFIX_EnableDepthWriting);
#define depth_clear
// depth_clear() - Clears the depth buffer that is currently in use.
//                 As of Surface Fix 1.2, this function is obsolete and is no longer necessary to use.

external_call(global.__SFIX_ClearDepthBuffer);
