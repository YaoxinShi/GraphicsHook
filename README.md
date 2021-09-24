# GraphicsHook

Execute:

- Run dx11app.exe
- Run "Inject.exe dx11app.exe DirectX11Hook.dll"
- Press "Insert" key, you can switch the UI on/off

Code:

- Inject - https://github.com/hiitiger/goverlay/tree/master/game-overlay
- Hook - https://github.com/hiitiger/goverlay/tree/master/game-overlay
- DirectX11Hook - https://github.com/niemand-sec/DirectX11Hook
- dx11app - https://github.com/walbourn/directx-sdk-samples/tree/master/Direct3D11Tutorials/Tutorial04

Inject method:
- CreateRemoteThread
- SetWindowsHookEx

Hook method:
- Detours