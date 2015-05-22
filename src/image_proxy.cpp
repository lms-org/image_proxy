#include <dirent.h>
#include <sys/types.h>
#include <algorithm>

#include "image_proxy.h"
#include "lms/imaging/pnm.h"

bool ImageProxy::initialize() {
    config = getConfig();
    inputImage = datamanager()
            ->readChannel<lms::imaging::Image>(this, "INPUT_IMAGE");
    outputImage = datamanager()
            ->writeChannel<lms::imaging::Image>(this, "OUTPUT_IMAGE");

    displayMode = DIRECTORY;
    playMode = PLAY;

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
    for(const std::string &msg : messaging()->receive("image_proxy")) {
        if(msg == "togglePlayMode") {
            playMode = playMode == PLAY ? STOP : PLAY;
        } else if(msg == "nextImage") {
            if(displayMode == DIRECTORY && playMode == STOP) { // stop mode is manual mode
                if(dirFilesIndex + 1 == dirFiles.size()) {
                    dirFilesIndex = 0;
                } else {
                    dirFilesIndex++;
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
}

void ImageProxy::imageChannelMode() {
    // only if in live mode then copy the input image channel
    if(playMode == PLAY) {
        *outputImage = *inputImage;
    }
}

void ImageProxy::singleFileMode() {
    std::string singleFile = config->get<std::string>("singleFile");

    if(singleFile.empty()) {
        logger.warn("singleFileMode") << "Property singleFile is not specified";
        return;
    }

    bool readRes = lms::imaging::readPNM(*outputImage, singleFile);

    if(! readRes) {
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

    std::string newDirectory = config->get<std::string>("directory");
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
