"C:\Program Files\VulkanSDK\1.4.309.0\Bin\glslc.exe" "shader.vert" -o "vert.spv"
"C:\Program Files\VulkanSDK\1.4.309.0\Bin\glslc.exe" "shader.frag" -o "frag.spv"
@echo off
echo Set WshShell = WScript.CreateObject("WScript.Shell") > %tmp%\tmp.vbs
echo WScript.Quit (WshShell.Popup("Success!", 0, "Message Title", 0)) >> %tmp%\tmp.vbs
cscript //nologo %tmp%\tmp.vbs
del %tmp%\tmp.vbs