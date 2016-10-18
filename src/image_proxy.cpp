#include <dirent.h>
#include <sys/types.h>
#include <algorithm>

#include "image_proxy.h"
#include "lms/imaging/pnm.h"

bool ImageProxy::initialize() {
    inputImage = readChannel<lms::imaging::Image>("INPUT_IMAGE");
    outputImage = writeChannel<lms::imaging::Image>("OUTPUT_IMAGE");

    std::string displayModeString = config().get<std::string>("displayMode");

    if(! displayModeFromString(displayModeString, displayMode)) {
        logger.warn("init") << "Invalid property displayMode: " <<
                               displayModeString;
        displayMode = IMAGE_CHANNEL;
    }

    playMode = PLAY;

    maxBufferSize = config().get<int>("maxBufferSize", 1000);

    return true;
}

bool ImageProxy::deinitialize() {
    return true;
}

bool ImageProxy::cycle() {
    fetchMessages();

    switch(displayMode) {
    case IMAGE_CHANNEL:
        imageChannelMode();
        break;
    case SINGLE_FILE:
        singleFileMode();
        break;
    case DIRECTORY:
        directoryMode();
        break;
    }

    return true;
}

void ImageProxy::fetchMessages() {
    /* TODO
    for(const std::string &msg : messaging()->receive("image_proxy")) {
        if(msg == "togglePlayMode") {
            playMode = playMode == PLAY ? STOP : PLAY;

            if(displayMode == IMAGE_CHANNEL) {
                // reset buffer index and future buffer
                bufferIndex = 0;
                futureBuffer.clear();
            }
        } else if(msg == "nextImage") {
            if(displayMode == DIRECTORY && playMode == STOP) { // stop mode is manual mode
                if(dirFilesIndex + 1 == dirFiles.size()) {
                    dirFilesIndex = 0;
                } else {
                    dirFilesIndex++;
                }
            }

            if(displayMode == IMAGE_CHANNEL && playMode == STOP) {
                if(bufferIndex + 1 > (int)futureBuffer.size()) {
                    bufferIndex = futureBuffer.size();
                } else {
                    bufferIndex ++;
                }
            }
        } else if(msg == "previousImage") {
            if(displayMode == DIRECTORY && playMode == STOP) { // stop mode is manual mode
                if(dirFilesIndex == 0) {
                    dirFilesIndex = dirFiles.size() - 1;
                } else {
                    dirFilesIndex--;
                }
            }

            if(displayMode == IMAGE_CHANNEL && playMode == STOP) {
                if(bufferIndex - 1 <= - (int)historyBuffer.size()) {
                    bufferIndex = - (int)historyBuffer.size();
                } else {
                    bufferIndex --;
                }
            }
        } else if(msg == "changeDisplayMode") {
            // rotate through display modes
            switch(displayMode) {
            case SINGLE_FILE:
                displayMode = DIRECTORY;
                break;
            case DIRECTORY:
                displayMode = IMAGE_CHANNEL;
                break;
            case IMAGE_CHANNEL:
                displayMode = SINGLE_FILE;
                break;
            }
        }
    }
    */
}

void ImageProxy::imageChannelMode() {
    if(inputImage->size() == 0) {
        drawFailImage(*outputImage);
        return;
    } else {
        if(playMode == STOP) {
            // save current image in future buffer
            if(futureBuffer.size() < maxBufferSize) {
                futureBuffer.push_back(*inputImage);
            }
        } else {
            // save current image in history buffer
            historyBuffer.push_front(*inputImage);

            if(historyBuffer.size() > maxBufferSize) {
                historyBuffer.pop_back();
            }
        }
    }

    // only if in live mode then copy the input image channel
    if(playMode == PLAY) {
        *outputImage = *inputImage;
    } else {
        // use image from ring buffer
        if(bufferIndex <= 0) {
            *outputImage = historyBuffer[- bufferIndex];
        } else {
            *outputImage = futureBuffer[bufferIndex - 1];
        }
    }
}

void ImageProxy::singleFileMode() {
    std::string singleFile = config().get<std::string>("singleFile");

    if(singleFile.empty()) {
        logger.warn("singleFileMode") << "Property singleFile is not specified";
        return;
    }

    bool readRes = lms::imaging::readPNM(*outputImage, singleFile);

    if(! readRes) {
        drawFailImage(*outputImage);
        logger.error("singleFileMode") << "Could not read image: " << singleFile;
    }
}

void ImageProxy::directoryMode() {
    if(playMode == PLAY) {
        if(dirFilesIndex + 1 == dirFiles.size()) {
            dirFilesIndex = 0;
        } else {
            dirFilesIndex++;
        }
    }

    std::string newDirectory = config().get<std::string>("directory");
    if(newDirectory != directory) {
        directory = newDirectory;

        dirFiles.clear();
        loadDirectory(directory, dirFiles);
    }

    if(dirFiles.empty()) {
        logger.warn("directoryMode") << "Directory is empty: " << directory;
        return;
    }

    std::string filePath = directory + "/" + dirFiles[dirFilesIndex];
    bool readRes = lms::imaging::readPNM(*outputImage, filePath);

    if(! readRes) {
        drawFailImage(*outputImage);
        logger.error("directoryMode") << "Could not read image: " << filePath;
    }
}

void ImageProxy::loadDirectory(const std::string &dir,
                                      std::vector<std::string>& files) {
    DIR *fd = opendir(dir.c_str());
    struct dirent *ent;

    if(fd == nullptr) {
        return;
    }

    while((ent = readdir(fd)) != nullptr) {
        if(ent->d_type == DT_REG) {
            std::string filename = ent->d_name;
            files.push_back(filename);
        }
    }

    closedir(fd);

    std::sort(files.begin(), files.end());
}

void ImageProxy::drawFailImage(lms::imaging::Image &image) {
    if(image.size() == 0) {
        image.resize(500, 500, lms::imaging::Format::GREY);
    }

    image.fill(255);
}

bool ImageProxy::displayModeFromString(const std::string &s, DisplayMode &mode) {
    if(s == "SINGLE_FILE") {
        mode = SINGLE_FILE;
        return true;
    }
    if(s == "DIRECTORY") {
        mode = DIRECTORY;
        return true;
    }
    if(s == "IMAGE_CHANNEL") {
        mode = IMAGE_CHANNEL;
        return true;
    }

    return false;
}
