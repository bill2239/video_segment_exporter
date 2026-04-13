# Video Segment Exporter

A Qt-based desktop application for selecting, viewing, and exporting video segments in various formats.

## Features

- **Video/Image Playback**: Load and play video files (MP4, MOV, AVI, MKV, WebM, M4V) or browse image directories
- **Segment Selection**: Interactive timeline widget to select start and end frames of video segments
- **Frame Export**: Export selected frames as individual JPEG images
- **Video Export**: Export selected segments as MP4 video files
- **GIF Export**: Convert selected video segments into animated GIF files
- **Real-time Playback**: Play videos with adjustable frame rate

## Requirements

- Qt6 (Widgets, Core, GUI, Concurrent)
- OpenCV
- CMake 3.16+
- C++17 compiler

## Building

### Quick Build

```bash
./build.sh
```

### Manual Build

```bash
mkdir build
cd build
cmake ..
make -j4
./VideoSegmentSelector
```

## Usage

1. **Load Media**: Click "Load Video / Directory" to open a video file or load a video from a directory of images
2. **Select Segment**: Use the timeline widget to drag the red handles and select your desired start and end frames
3. **Play/Pause**: Click "Pause" to toggle playback
4. **Export Options**:
   - **Export Frames**: Save selected frames as individual JPEG files
   - **Export Video Segment**: Create an MP4 video from selected frames
   - **Frames → GIF**: Convert selected frames to an animated GIF

## Project Structure

- `main.cpp` - Application entry point
- `video_player.h/cpp` - Main video playback and export functionality
- `segment_selector.h/cpp` - Interactive timeline widget for frame selection
- `gif.h/cpp` - GIF encoding implementation
- `CMakeLists.txt` - Build configuration

## Technical Details

### Supported Input Formats
- **Video**: MP4, MOV, AVI, MKV, WebM, M4V
- **Images**: Any format supported by OpenCV in a directory

### Export Formats
- **Frames**: JPEG
- **Video**: MP4 (H.264 codec)
- **Animation**: GIF with configurable frame duration

### Key Technologies
- **Qt6**: GUI framework
- **OpenCV**: Video processing and image handling
- **GIF Encoder**: Custom implementation for efficient GIF creation with dithering support

## Notes

- The GIF export runs asynchronously to avoid blocking the UI
- Frame rate can be adjusted during playback
- Supports both video files and image sequences
- The segment selector provides visual feedback for frame range selection

If you find this repo useful to you please consider click the button below to donate and support my work!
[![Buy Me A Coffee](https://www.buymeacoffee.com/assets/img/custom_images/yellow_img.png)](https://buymeacoffee.com/bill2239)