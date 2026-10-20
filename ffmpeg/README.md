# FFmpeg runtime files

Radit uses FFmpeg for audio encoding and streaming. FFmpeg binaries are intentionally kept out of this source repository: the executable and its libraries differ by operating system and architecture.

## Windows

1. Download a Windows FFmpeg build from the [FFmpeg download page](https://ffmpeg.org/download.html) or one of the Windows build providers linked there.
2. Copy `ffmpeg.exe` and every DLL included with that build into this `ffmpeg` directory in the Radit runtime folder.
3. Keep the directory beside `radit.exe`, so the resulting path is `ffmpeg/ffmpeg.exe`.

Do not copy only `ffmpeg.exe`; its accompanying DLLs may be required. Do not commit the binaries to this repository.

## Linux

Do not use the Windows executable or DLLs. Use the Linux Radit package for a ready-to-run build. Linux source-build FFmpeg integration is not documented here yet.
