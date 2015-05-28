# image_proxy

LMS module that loads image from a folder, a single file or uses an image from an input data channel. Features:
- Stop, go back and replay image from a directory
- Change display mode between SINGLE_FILE, DIRECTORY and IMAGE_CHANNEL
- Save input data channel in a history buffer
- Change play mode between playing and stopping for the input data channel
- If in stop mode the input data channel is saved in future buffer

## Data channels
- **INPUT_IMAGE** - image from a camera (or something else)
- **OUTPUT_IMAGE** - the loaded image is stored in this data channel

## Messaging
- **togglePlayMode** - toogle between PLAY and STOP
- **nextImage** - go to next image in STOP mode
- **previousImage** - go to previous image in STOP mode
- **changeDisplayMode** - change display mode

## Config
- **displayMode** - initial display mode, can be SINGLE_FILE, DIRECTORY or IMAGE_CHANNEL
- **singleFile** - absolute path to a single image file
- **directory** - absolute path to a folder containing image files

## Dependencies
- [imaging](https://github.com/syxolk/imaging)

## Recommended modules
- [camera_importer](https://github.com/Phibedy/camera_importer)
