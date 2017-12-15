/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2017                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include "torrentsynchronization.h"

#include <modules/sync/syncmodule.h>

#include <openspace/documentation/verifier.h>
#include <openspace/engine/openspaceengine.h>
#include <openspace/engine/moduleengine.h>


#include <ghoul/logging/logmanager.h>
#include <ghoul/filesystem/filesystem.h>

#include <fstream>


namespace {
    const char* _loggerCat = "TorrentSynchronization";
    const char* KeyIdentifier = "Identifier";
    const char* KeyMagnet = "Magnet";
}

namespace openspace {

TorrentSynchronization::TorrentSynchronization(const ghoul::Dictionary& dict,
                                               const std::string& synchronizationRoot,
                                               TorrentClient* torrentClient)
    : openspace::ResourceSynchronization()
    , _synchronizationRoot(synchronizationRoot)
    , _torrentClient(torrentClient)
{
    documentation::testSpecificationAndThrow(
        Documentation(),
        dict,
        "TorrentSynchroniztion"
    );

    _identifier = dict.value<std::string>(KeyIdentifier);
    _magnetLink = dict.value<std::string>(KeyMagnet);
}

TorrentSynchronization::~TorrentSynchronization() {
    cancel();
}

documentation::Documentation TorrentSynchronization::Documentation() {
    using namespace openspace::documentation;
    return {
        "TorrentSynchronization",
        "torrent_synchronization",
        {
            {
                KeyIdentifier,
                new StringVerifier,
                Optional::No,
                "A unique identifier for this torrent"
            },
            {
                KeyMagnet,
                new StringVerifier,
                Optional::No,
                "A magnet link identifying the torrent"
            }
        }
    };
}

std::string TorrentSynchronization::uniformResourceName() const {
    size_t begin = _magnetLink.find("=urn") + 1;
    size_t end = _magnetLink.find('&', begin);
    std::string xs = _magnetLink.substr(begin, end == std::string::npos ? end : (end - begin));
    std::transform(xs.begin(), xs.end(), xs.begin(), [](char x) {
        if (x == ':') return '.';
        return x;
    });
    return xs;
}

std::string TorrentSynchronization::directory() {
    ghoul::filesystem::Directory d(
        _synchronizationRoot +
        ghoul::filesystem::FileSystem::PathSeparator +
        "torrent" +
        ghoul::filesystem::FileSystem::PathSeparator +
        _identifier +
        ghoul::filesystem::FileSystem::PathSeparator +
        uniformResourceName()
    );

    return FileSys.absPath(d);
}

void TorrentSynchronization::start() {
    if (_enabled) {
        return;
    }
    begin();

    if (hasSyncFile()) {
        resolve();
    }

    _enabled = true;
    _torrentId = _torrentClient->addMagnetLink(
        _magnetLink,
        directory(),
        [this](TorrentClient::TorrentProgress p) {
            updateTorrentProgress(p);
        }
    );
}

void TorrentSynchronization::cancel() {
    if (_enabled) {
        _torrentClient->removeTorrent(_torrentId);
        _enabled = false;
        reset();
    }
}

void TorrentSynchronization::clear() {
    // Todo: remove directory!
    cancel();
}

bool TorrentSynchronization::hasSyncFile() {
    std::string path = directory() + ".ossync";
    return FileSys.fileExists(path);
}

void TorrentSynchronization::createSyncFile() {
    std::string dir = directory();
    std::string filepath = dir + ".ossync";
    FileSys.createDirectory(dir, ghoul::filesystem::Directory::Recursive::Yes);
    std::ofstream syncFile(filepath, std::ofstream::out);
    syncFile << "Synchronized";
    syncFile.close();
}

size_t TorrentSynchronization::nSynchronizedBytes() {
    std::lock_guard<std::mutex> g(_progressMutex);
    return _progress.nDownloadedBytes;
}

size_t TorrentSynchronization::nTotalBytes() {
    std::lock_guard<std::mutex> g(_progressMutex);
    return _progress.nTotalBytes;
}

bool TorrentSynchronization::nTotalBytesIsKnown() {
    std::lock_guard<std::mutex> g(_progressMutex);
    return _progress.nTotalBytesKnown;
}

void TorrentSynchronization::updateTorrentProgress(TorrentClient::TorrentProgress progress) {
    std::lock_guard<std::mutex> g(_progressMutex);
    _progress = progress;
    if (progress.finished && state() == State::Syncing) {
        createSyncFile();
        resolve();
    }
}

} // namespace openspace
