// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * A class that takes care of communication with NaCL and creates an archive.
 * One instance of this class is created for each pack request. Since multiple
 * compression requests can be in progress at the same time, each instance has
 * a unique compressor id of positive integer. Every communication with NaCL
 * must be done with compressor id.
 * @constructor
 * @param {!Object} naclModule The nacl module.
 * @param {!Array} items The items to be packed.
 */
unpacker.Compressor = function(naclModule, items) {
  /**
   * @private {!Object}
   * @const
   */
  this.naclModule_ = naclModule;

  /**
   * @private {!Array}
   * @const
   */
  this.items_ = items;

  /**
   * @private {!unpacker.types.CompressorId}
   * @const
   */
  this.compressorId_ = unpacker.Compressor.compressorIdCounter++;

  /**
   * @private {string}
   * @const
   */
  this.archiveName_ = this.getArchiveName_();

  /**
   * The counter used to assign a unique id to each entry.
   * @type {number}
   */
  this.entryIdCounter_ = 1;

  /**
   * The set of entry ids waiting for metadata from FileSystem API.
   * These requests needs to be tracked here to tell whether all pack process
   * has finished or not.
   * @type {!Set}
   */
  this.metadataRequestsInProgress_ = new Set();

  /**
   * The queue containing entry ids that have already obtained metadata from
   * FileSystem API and are waiting to be added into archive.
   * @type {!Array}
   */
  this.pendingAddToArchiveRequests_ = [];

  /**
   * The id of the entry that is being compressed and written into archive.
   * Note that packing of each entry should be done one by one unlike
   * unpacking. Thus, at most one entry is processed at once.
   * @type {!unpacker.types.EntryId}
   */
  this.entryIdInProgress_ = 0;

  /**
   * Map from entry ids to entries.
   * @const {!Object<!unpacker.types.EntryId, !FileEntry|!DirectoryEntry>}
   */
  this.entries_ = {};

  /**
   * Map from entry ids to its metadata.
   * @const {!Object<!unpacker.types.EntryId, !Metadata>}
   */
  this.metadata_ = {};

  /**
   * The offset from which the entry in progress should be read.
   * @type {number}
   */
  this.offset_ = 0;
};

/**
 * The counter which is assigned and incremented every time a new compressor
 * instance is created.
 * @type {number}
 */
unpacker.Compressor.compressorIdCounter = 1;

/**
 * The queue containing compressor ids that wait for foreground page to be
 * loaded. Once this extension becomes a component extension, we don't need to
 * create an archive file on the foreground page and this also gets unnecessary.
 * @type {!Array}
 */
unpacker.Compressor.CompressorIdQueue = [];

/**
 * The default archive name.
 * @type {string}
 */
unpacker.Compressor.DEFAULT_ARCHIVE_NAME = 'Archive.zip';

/**
 * The getter function for compressor id.
 * @return {!unpacker.types.CompressorId}
 */
unpacker.Compressor.prototype.getCompressorId = function() {
  return this.compressorId_;
};

/**
 * Returns the archive file name.
 * @private
 * @return {string}
 */
unpacker.Compressor.prototype.getArchiveName_ = function() {
  // When multiple entries are selected.
  if (this.items_.length !== 1)
    return unpacker.Compressor.DEFAULT_ARCHIVE_NAME;

  var name = this.items_[0].entry.name;
  var idx = name.lastIndexOf('.');
  // When the name does not have extension.
  // TODO(takise): This converts file.tar.gz to file.tar.zip.
  if (idx === -1)
    return name + '.zip';
  // When the name has extension.
  return name.substring(0, idx) + '.zip';
};

/**
 * Starts actual compressing process.
 * Creates an archive file and requests minizip to create an archive object.
 * @param {function(!unpacker.types.CompressorId)} onSuccess
 * @param {function(!unpacker.types.CompressorId)} onError
 */
unpacker.Compressor.prototype.compress = function(onSuccess, onError) {
  this.onSuccess_ = onSuccess;
  this.onError_ = onError;

  this.getArchiveFile_();
};

/**
 * Gets an archive file with write permission. Currently, this extension does
 * not have permission to create files from the background page. Thus, this
 * function first creates a foreground page and then creates an archive file in
 * it. Once this extension becomes a component extension, this process will be
 * simpler.
 * @private
 */
unpacker.Compressor.prototype.getArchiveFile_ = function() {
  // If the foreground page already exists, create an archive file.
  if (this.createArchiveFileForeground_) {
    this.createArchiveFileForeground_(this.compressorId_);
  } else {
    // If the foreground page does not exist, push the id of this compressor to
    // the queue so that we can resume later and create the foreground page.
    // We need this queue because multiple compressors can wait for the
    // foreground page to be loaded.
    var queue = unpacker.Compressor.CompressorIdQueue;
    queue.push(this.compressorId_);
    if (queue.length === 1) {
      chrome.app.window.create('../html/compressor.html', {hidden: true});
    }
  }
};

/**
 * Sends an create archive request to NaCL.
 * @private
 */
unpacker.Compressor.prototype.sendCreateArchiveRequest_ = function() {
  var request = unpacker.request.createCreateArchiveRequest(this.compressorId_);
  this.naclModule_.postMessage(request);
};

/**
 * A handler of create archive done response.
 * Enumerates entries and requests FileSystem API for their metadata.
 * @private
 */
unpacker.Compressor.prototype.createArchiveDone_ = function() {
  this.items_.forEach(function(item) {
    this.getEntryMetadata_(item.entry);
  }.bind(this));
};

/**
 * Gets metadata of a file or directory.
 * @param {!FileEntry|!DirectoryEntry} entry FileEntry or DirectoryEntry.
 * @private
 */
unpacker.Compressor.prototype.getEntryMetadata_ = function(entry) {
  if (entry.isFile)
    this.getSingleMetadata_(entry);
  else
    this.getDirectoryEntryMetadata_(/** @type {!DirectoryEntry} */ (entry));
};

/**
 * Requests metadata of an entry non-recursively.
 * @param {!FileEntry|!DirectoryEntry} entry FileEntry or DirectoryEntry.
 * @private
 */
unpacker.Compressor.prototype.getSingleMetadata_ = function(entry) {
  var entryId = this.entryIdCounter_++;
  this.metadataRequestsInProgress_.add(entryId);
  this.entries_[entryId] = entry;

  entry.getMetadata(
      function(metadata) {
        this.metadataRequestsInProgress_.delete(entryId);
        this.pendingAddToArchiveRequests_.push(entryId);
        this.metadata_[entryId] = metadata;
        this.sendAddToArchiveRequest_();
      }.bind(this),
      function(error) {
        console.error('Failed to get metadata: ' + error.message + '.');
        this.onError_(this.compressorId_);
      }.bind(this));
};

/**
 * Requests metadata of an entry recursively.
 * @param {!DirectoryEntry} dir DirectoryEntry.
 * @private
 */
unpacker.Compressor.prototype.getDirectoryEntryMetadata_ = function(dir) {

  // Read entries in dir and call getEntryMetadata_ for them recursively.
  var dirReader = dir.createReader();

  // Recursive function
  var getEntries = function() {
    dirReader.readEntries(
        function(results) {
          // ReadEntries must be called until it returns nothing, because
          // it does not necessarily return all entries in the directory.
          if (results.length) {
            results.forEach(this.getEntryMetadata_.bind(this));
            getEntries();
          }
        }.bind(this),
        function(error) {
          console.error(
              'Failed to get directory entries: ' + error.message + '.');
          this.onError_(this.compressorId_);
        }.bind(this));
  }.bind(this);

  getEntries();

  // Get the metadata of this dir itself.
  this.getSingleMetadata_(dir);
};

/**
 * Pops an entry from the queue and adds it to the archive.
 * If another entry is in progress, this function does nothing. If there is no
 * entry in the queue, it shifts to close archive process. Otherwise, this sends
 * an add to archive request for a popped entry with its metadata to minizip.
 * @private
 */
unpacker.Compressor.prototype.sendAddToArchiveRequest_ = function() {
  // Another process is in progress.
  if (this.entryIdInProgress_ != 0)
    return;

  // All entries have already been archived.
  if (this.pendingAddToArchiveRequests_.length === 0) {
    if (this.metadataRequestsInProgress_.size === 0)
      this.sendCloseArchiveRequest(false /* hasError */);
    return;
  }

  var entryId = this.pendingAddToArchiveRequests_.shift();
  this.entryIdInProgress_ = entryId;

  // Convert the absolute path on the virtual filesystem to a relative path from
  // the archive root by removing the leading '/' if exists.
  var fullPath = this.entries_[entryId].fullPath;
  if (fullPath.length && fullPath[0] == '/')
    fullPath = fullPath.substring(1);

  // Modification time is set to the archive in local time.
  var utc = this.metadata_[entryId].modificationTime;
  var modificationTime = utc.getTime() - (utc.getTimezoneOffset() * 60000);

  var request = unpacker.request.createAddToArchiveRequest(
      this.compressorId_, entryId, fullPath, this.metadata_[entryId].size,
      modificationTime, this.entries_[entryId].isDirectory);
  this.naclModule_.postMessage(request);
};

/**
 * Sends a close archive request to minizip. minizip writes metadata of
 * the archive itself on the archive and releases objects obtainted in the
 * packing process.
 */
unpacker.Compressor.prototype.sendCloseArchiveRequest = function(hasError) {
  var request =
      unpacker.request.createCloseArchiveRequest(this.compressorId_, hasError);
  this.naclModule_.postMessage(request);
};

/**
 * Sends a read file chunk done response.
 * @param {number} length The number of bytes read from the entry.
 * @param {!ArrayBuffer} buffer A buffer containing the data that was read.
 * @private
 */
unpacker.Compressor.prototype.sendReadFileChunkDone_ = function(
    length, buffer) {
  var request = unpacker.request.createReadFileChunkDoneResponse(
      this.compressorId_, length, buffer);
  this.naclModule_.postMessage(request);
};

/**
 * A handler of read file chunk messages.
 * Reads 'length' bytes from the entry currently in process.
 * @param {!Object} data
 * @private
 */
unpacker.Compressor.prototype.onReadFileChunk_ = function(data) {
  var entryId = this.entryIdInProgress_;
  var entry = this.entries_[entryId];
  var length = Number(data[unpacker.request.Key.LENGTH]);

  // A function to create a reader and read bytes.
  var readFileChunk = function() {
    var file = this.file_.slice(this.offset_, this.offset_ + length);
    var reader = new FileReader();

    reader.onloadend = function(event) {
      var buffer = event.target.result;

      // The buffer must have 'length' bytes because the byte length which can
      // be read from the file is already calculated on NaCL side.
      if (buffer.byteLength !== length) {
        console.error(
            'Tried to read chunk with length ' + length +
            ', but byte with length ' + buffer.byteLength + ' was returned.');

        // If the first argument(length) is negative, it means that an error
        // occurred in reading a chunk.
        this.sendReadFileChunkDone_(-1, buffer);
        this.onError_(this.compressorId_);
        return;
      }

      this.offset_ += length;
      this.sendReadFileChunkDone_(length, buffer);
    }.bind(this);

    reader.onerror = function(event) {
      console.error(
          'Failed to read file chunk. Name: ' + file.name +
          ', offset: ' + this.offset_ + ', length: ' + length + '.');

      // If the first argument(length) is negative, it means that an error
      // occurred in reading a chunk.
      this.sendReadFileChunkDone_(-1, new ArrayBuffer(0));
      this.onError_(this.compressorId_);
    };

    reader.readAsArrayBuffer(file);
  }.bind(this);

  // When the entry is read for the first time.
  if (!this.file_) {
    entry.file(function(file) {
      this.file_ = file;
      readFileChunk();
    }.bind(this));
    return;
  }

  // From the second time onward.
  readFileChunk();
};

/**
 * A handler of write chunk requests.
 * Writes the data in the given buffer onto the archive file.
 * @param {!Object} data
 * @private
 */
unpacker.Compressor.prototype.onWriteChunk_ = function(data) {
  var offset = Number(data[unpacker.request.Key.OFFSET]);
  var length = Number(data[unpacker.request.Key.LENGTH]);
  var buffer = data[unpacker.request.Key.CHUNK_BUFFER];
  this.writeChunk_(offset, length, buffer, this.sendWriteChunkDone_.bind(this));
};

/**
 * Writes buffer into the archive file (window.archiveFileEntry).
 * @param {number} offset The offset from which date is written.
 * @param {number} length The number of bytes in the buffer to write.
 * @param {!ArrayBuffer} buffer The buffer to write in the archive.
 * @param {function(number)} callback Callback to execute at the end of the
 *     function. This function has one parameter: length, which represents the
 *     length of bytes written on to the archive. If writing a chunk fails,
 *     a negative value must be assigned to this argument.
 * @private
 */
unpacker.Compressor.prototype.writeChunk_ = function(
    offset, length, buffer, callback) {
  // TODO(takise): Use the same instance of FileWriter over multiple calls of
  // this function instead of creating new ones.
  this.archiveFileEntry_.createWriter(
      function(fileWriter) {
        fileWriter.onwriteend = function(event) {
          callback(length);
        };

        fileWriter.onerror = function(event) {
          console.error(
              'Failed to write chunk to ' + this.archiveFileEntry_ + '.');

          // If the first argument(length) is negative, it means that an error
          // occurred in writing a chunk.
          callback(-1 /* length */);
          this.onError_(this.compressorId_);
        };

        // Create a new Blob and append it to the archive file.
        var blob = new Blob([buffer], {});
        fileWriter.seek(offset);
        fileWriter.write(blob);
      },
      function(event) {
        console.error(
            'Failed to create writer for ' + this.archiveFileEntry_ + '.');
        this.onError_(this.compressorId_);
      });
};

/**
 * Sends a write chunk done response.
 * @param {number} length The number of bytes written onto the entry.
 * @private
 */
unpacker.Compressor.prototype.sendWriteChunkDone_ = function(length) {
  var request =
      unpacker.request.createWriteChunkDoneResponse(this.compressorId_, length);
  this.naclModule_.postMessage(request);
};

/**
 * A handler of add to archive done responses.
 * Resets information on the current entry and starts processing another entry.
 * @private
 */
unpacker.Compressor.prototype.onAddToArchiveDone_ = function() {
  // Reset information on the current entry.
  this.entryIdInProgress_ = 0;
  this.file_ = null;
  this.offset_ = 0;

  // Start processing another entry.
  this.sendAddToArchiveRequest_();
};

/**
 * A handler of close archive responses.
 * Receiving this response means the entire packing process has finished.
 * @private
 */
unpacker.Compressor.prototype.onCloseArchiveDone_ = function() {
  this.onSuccess_(this.compressorId_);
};

/**
 * Processes messages from NaCl module.
 * @param {!Object} data The data contained in the message from NaCl. Its
 *     types depend on the operation of the request.
 * @param {!unpacker.request.Operation} operation An operation from request.js.
 */
unpacker.Compressor.prototype.processMessage = function(data, operation) {
  switch (operation) {
    case unpacker.request.Operation.CREATE_ARCHIVE_DONE:
      this.createArchiveDone_();
      break;

    case unpacker.request.Operation.READ_FILE_CHUNK:
      this.onReadFileChunk_(data);
      break;

    case unpacker.request.Operation.WRITE_CHUNK:
      this.onWriteChunk_(data);
      break;

    case unpacker.request.Operation.ADD_TO_ARCHIVE_DONE:
      this.onAddToArchiveDone_();
      break;

    case unpacker.request.Operation.CLOSE_ARCHIVE_DONE:
      this.onCloseArchiveDone_();
      break;

    case unpacker.request.Operation.COMPRESSOR_ERROR:
      console.error(
          'Compressor error for compressor id ' + this.compressorId_ + ': ' +
          data[unpacker.request.Key.ERROR]);  // The error contains
                                              // the '.' at the end.
      this.onError_(this.compressorId_);
      break;

    default:
      console.error('Invalid NaCl operation: ' + operation + '.');
      this.onError_(this.compressorId_);
  }
};
