//=========
// Author:  Alex Reynolds & Shane Neph
// Project: starch
// File:    starchApi.hpp
//=========

//
//    BEDOPS
//    Copyright (C) 2011, 2012, 2013 Shane Neph, Scott Kuehn and Alex Reynolds
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#ifndef STARCH_API_H
#define STARCH_API_H

#include "starchMetadataHelpers.h"
#include "starchFileHelpers.h"
#include "starchHelpers.h"
#include "unstarchHelpers.h"
#include "data/starch/starchConstants.h"

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <bzlib.h>
#include <zlib.h>

namespace starch 
{
    class Starch 
    {
        public:
            Starch();
            explicit Starch(FILE *, const std::string& _chr = "all", bool _perLineUsage = true);
            explicit Starch(const std::string&);
            Starch(FILE *, const std::string&, const std::string&, bool);
            Starch(const std::string&, const bool);
            Starch(const std::string&, const bool, const bool, const std::string&);
            Starch(const std::string&, const bool, const bool, Metadata *, CompressionType, ArchiveVersion *, uint64_t, unsigned int);
            virtual ~Starch();
            Starch(const Starch& cpArchive);            
            Starch& operator=(const Starch& cpArchive);

            int listJSONMetadata(FILE *out, FILE *err);
            bool extractBEDLine(std::string& line);
            int extractAllData(const std::string& chr, FILE *out);

            static bool fnExists(const std::string& _inFn) 
            {
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::fnExists(std::string) ---\n");
#endif
                struct stat _buf;
                return (stat(_inFn.c_str(), &_buf) == 0);
            }

            static bool fpIsOpen(FILE *_inFp)
            {
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::fpIsOpen(FILE *) ---\n");
#endif
                if (std::fseek(_inFp, 0, SEEK_SET) != 0) 
                    return false;

                if (std::ftell(_inFp) != 0L) 
                    return false;

                return true;
            }

            static bool hasStarchRevision2Header(FILE *_inFp)
            {
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::hasStarchRevision2Header(FILE *) ---\n");
#endif
                int _testByte;
                size_t _testIdx;

                for (_testIdx = 0; _testIdx < sizeof(starchRevision2HeaderBytes); _testIdx++) {
                    _testByte = std::fgetc(_inFp);
                    if (_testByte != starchRevision2HeaderBytes[_testIdx]) {
                        std::fseek(_inFp, 0, SEEK_SET);
                        return false;
                    }
                }

                if (std::fseek(_inFp, 0, SEEK_SET) != 0) 
                    return false;

                return true;
            }

            static bool hasStarchRevision2Header(std::string& _inFn)
            {
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::hasStarchRevision2Header(std::string) ---\n");
#endif
                FILE *_testFp = NULL;

                _testFp = std::fopen(_inFn.c_str(), "r");
                if (!_testFp) 
                    return false;

                if (!hasStarchRevision2Header(_testFp)) {
                    std::fclose(_testFp);
                    return false;
                }

                if (std::fclose(_testFp) != 0)
                    return false;

                return true;
            }

            static bool hasStarchRevision1Header(FILE *_inFp)
            {
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::hasStarchRevision1Header(FILE *) ---\n");
#endif
                int _testByte;
                size_t _testIdx;
                char _testMagicBuf[STARCH_TEST_BYTE_COUNT] = {0};
                uint64_t _mdOffset = UINT64_C(0);
                uint64_t _mdOffsetIdx = UINT64_C(0);
                int _testElemSize = sizeof(char);
                int _testElemCount = STARCH_TEST_BYTE_COUNT;
                char *_testMagicPrecursor = NULL;
                char *_mdBuf = NULL;
                json_t *_mdJSON = NULL;
                json_error_t _mdJSONError;
                json_t *_mdJSONArchive = NULL;
                const char *_mdJSONObjKey = NULL;
                json_t *_mdJSONObjValue = NULL;
                json_t *_mdJSONArchVersion = NULL;
                const char *_mdJSONArchVerObjKey = NULL;
                json_t *_mdJSONArchVerObjValue = NULL;
                ArchiveVersion *_archVersion = NULL;
                json_t *_mdJSONStreams = NULL;
                char *_archStreamChr = NULL;
                char *_archStreamFn = NULL;
                size_t _mdJSONStreamIdx;
                json_t *_mdJSONStream = NULL;
                json_t *_mdJSONStreamChr = NULL;
                json_t *_mdJSONStreamFn = NULL;
                json_t *_mdJSONStreamSize = NULL;
                json_t *_mdJSONStreamLineCount = NULL;
                json_t *_mdJSONStreamNonUniqueBaseCount = NULL;
                json_t *_mdJSONStreamUniqueBaseCount = NULL;
                uint64_t _archStreamSize = UINT64_C(0);
                Bed::LineCountType _archStreamLineCount = UINT64_C(0);
                Bed::BaseCountType _archStreamNonUniqueBaseCount = UINT64_C(0);
                Bed::BaseCountType _archStreamUniqueBaseCount = UINT64_C(0);
                Metadata *_testMd = NULL;
                Metadata *_firstMd = NULL;
                unsigned int _recIdx = 0U;
                size_t _nReadBytes = 0U;

                for (_testIdx = 0; _testIdx < sizeof(starchRevision1HeaderBytes); _testIdx++) {
                    _testByte = std::fgetc(_inFp);
                    if (_testByte != starchRevision1HeaderBytes[_testIdx]) {
                        std::fseek(_inFp, 0, SEEK_SET);
                        return false;
                    }
                }

                /* 
                    while we know that the file starts with the byte(s) 
                    that one will find in a v1 Starch archive, this could 
                    be a false positive, so we try to read the file until 
                    we reach the end of the metadata stored in the header.
                */

                _testMagicPrecursor = (char *) std::malloc((size_t) mdTerminatorBytesLength);
                if (!_testMagicPrecursor) {
                    std::fseek(_inFp, 0, SEEK_SET);
                    return false;
                }
                do {
                    if ((std::memcmp(_testMagicBuf, bzip2MagicBytes, sizeof(bzip2MagicBytes) - 1) == 0) || 
                        (std::memcmp(_testMagicBuf, gzipMagicBytes, sizeof(gzipMagicBytes) - 1) == 0)) 
                    {
                        STARCH_fseeko(_inFp, (off_t) (_mdOffset - mdTerminatorBytesLength), SEEK_SET);
                        _nReadBytes = std::fread(_testMagicPrecursor, mdTerminatorBytesLength, mdTerminatorBytesLength, _inFp);
                        if (_nReadBytes != (size_t) (mdTerminatorBytesLength * mdTerminatorBytesLength)) {
#ifdef __cplusplus
                            /* why can't c++ standardize on a format specifier for a simple std::size_t ? */
                            std::fprintf(stderr, "ERROR: Bytes read ( %" PRIu64 " ) not equal to terminator byte length!\n",
                                         static_cast<uint64_t>(_nReadBytes));
#else
                            std::fprintf(stderr, "ERROR: Bytes read (%zu) not equal to terminator byte length!\n", _nReadBytes);
#endif
                            std::exit(EXIT_FAILURE);
                        }
                        if ((std::memcmp(_testMagicPrecursor, dynamicMdTerminatorBytes, sizeof(dynamicMdTerminatorBytes)) == 0) || 
                            (std::memcmp(_testMagicPrecursor, legacyMdTerminatorBytes, sizeof(legacyMdTerminatorBytes)) == 0) ||
                            (std::memcmp(_testMagicPrecursor, otherLegacyMdTerminatorBytes, sizeof(otherLegacyMdTerminatorBytes)) == 0) )
                            break;
                    }
                    _mdOffset += _testElemSize * _testElemCount;
                    STARCH_fseeko(_inFp, (off_t) (_mdOffset - STARCH_TEST_BYTE_POSITION_RESET), SEEK_SET);
                    _mdOffset -= STARCH_TEST_BYTE_POSITION_RESET;
                } while (std::fread(_testMagicBuf, _testElemSize, _testElemCount, _inFp));
                free(_testMagicPrecursor);
                _testMagicPrecursor = NULL;
                if (std::fseek(_inFp, 0, SEEK_SET) != 0) 
                    return false;
                _mdBuf = (char *) std::malloc((size_t) _mdOffset + 1);
                if (!_mdBuf) {
                    std::fseek(_inFp, 0, SEEK_SET);
                    return false;
                }
                do {
                    *(_mdBuf + _mdOffsetIdx++) = std::fgetc(_inFp);
                } while (_mdOffsetIdx < _mdOffset);

                _mdJSON = json_loads((const char *) _mdBuf, JSON_DISABLE_EOF_CHECK, &_mdJSONError);
                free(_mdBuf);
                _mdBuf = NULL;
                _mdJSONArchive = json_object_get(_mdJSON, STARCH_METADATA_STREAM_ARCHIVE_KEY);
                _archVersion = (ArchiveVersion *) std::malloc(sizeof(ArchiveVersion));
                if (!_archVersion) {
                    std::fseek(_inFp, 0, SEEK_SET);
                    return false;
                }
                if (_mdJSONArchive) {
                    json_object_foreach(_mdJSONArchive, _mdJSONObjKey, _mdJSONObjValue) {
                        if (std::strcmp(_mdJSONObjKey, STARCH_METADATA_STREAM_ARCHIVE_VERSION_KEY) == 0) {
                            _mdJSONArchVersion = json_object_get(_mdJSONArchive, STARCH_METADATA_STREAM_ARCHIVE_VERSION_KEY);
                            if (!_mdJSONArchVersion) {
                                std::fseek(_inFp, 0, SEEK_SET);
                                return false;
                            }
                            json_object_foreach(_mdJSONArchVersion, _mdJSONArchVerObjKey, _mdJSONArchVerObjValue) {
                                if (strcmp(_mdJSONArchVerObjKey, STARCH_METADATA_STREAM_ARCHIVE_VERSION_MAJOR_KEY) == 0)
                                    _archVersion->major = (int) json_integer_value(_mdJSONArchVerObjValue);
                                if (strcmp(_mdJSONArchVerObjKey, STARCH_METADATA_STREAM_ARCHIVE_VERSION_MINOR_KEY) == 0)
                                    _archVersion->minor = (int) json_integer_value(_mdJSONArchVerObjValue);
                                if (strcmp(_mdJSONArchVerObjKey, STARCH_METADATA_STREAM_ARCHIVE_VERSION_REVISION_KEY) == 0)
                                    _archVersion->revision = (int) json_integer_value(_mdJSONArchVerObjValue);
                            }
                        }
                    }
                }
                else {
                    /* 
                        JSON object is old and does not contain an
                        'archive' object. so we instead create a 
                        dummy archive object with defaults.
                    */
                    _archVersion->major = 1;
                    _archVersion->minor = 0;
                    _archVersion->revision = 0;
                }

                _archStreamChr = (char *) std::malloc(STARCH_STREAM_METADATA_MAX_LENGTH + 1);
                _archStreamFn = (char *) std::malloc(STARCH_STREAM_METADATA_MAX_LENGTH + 1);
                if (!_archStreamChr || !_archStreamFn) {
                    std::fseek(_inFp, 0, SEEK_SET);
                    return false;
                }

                if (((_archVersion->major == STARCH_MAJOR_VERSION ) && 
                     (_archVersion->minor <= STARCH_MINOR_VERSION) && 
                     (_archVersion->revision <= STARCH_REVISION_VERSION)) || 
                     (_archVersion->major < STARCH_MAJOR_VERSION))
                {
                    _mdJSONStreams = json_object_get(_mdJSON, STARCH_METADATA_STREAM_LIST_KEY);
                    if (!_mdJSONStreams) {
                        std::fprintf(stderr, "ERROR: Could not retrieve JSON streams list\n");
                        std::fseek(_inFp, 0, SEEK_SET);
                        return false;
                    }
                    for (_mdJSONStreamIdx = 0; _mdJSONStreamIdx < json_array_size(_mdJSONStreams); _mdJSONStreamIdx++) {
                        _mdJSONStream = json_array_get(_mdJSONStreams, _mdJSONStreamIdx);
                        if (!_mdJSONStream) {
                            std::fprintf(stderr, "ERROR: Could not retrieve JSON stream object\n");
                            std::fseek(_inFp, 0, SEEK_SET);
                            return false;
                        }
                        _mdJSONStreamChr = json_object_get(_mdJSONStream, STARCH_METADATA_STREAM_CHROMOSOME_KEY);
                        if (!_mdJSONStreamChr) {
                            std::fprintf(stderr, "ERROR: Could not retrieve JSON stream's chromosome attribute\n");
                            std::fseek(_inFp, 0, SEEK_SET);
                            return false;
                        }
                        std::strncpy(_archStreamChr, json_string_value(_mdJSONStreamChr), strlen(json_string_value(_mdJSONStreamChr)) + 1);
                        _mdJSONStreamFn = json_object_get(_mdJSONStream, STARCH_METADATA_STREAM_FILENAME_KEY);
                        if (!_mdJSONStreamFn) {
                            std::fprintf(stderr, "ERROR: Could not retrieve JSON stream's filename attribute\n");
                            std::fseek(_inFp, 0, SEEK_SET);
                            return false;
                        }
                        std::strncpy(_archStreamFn, json_string_value(_mdJSONStreamFn), strlen(json_string_value(_mdJSONStreamFn)) + 1);
                        _mdJSONStreamSize = json_object_get(_mdJSONStream, STARCH_METADATA_STREAM_SIZE_KEY);
                        if (!_mdJSONStreamSize) {
                            std::fprintf(stderr, "ERROR: Could not retrieve JSON stream's size attribute\n");
                            std::fseek(_inFp, 0, SEEK_SET);
                            return false;
                        }
                        _archStreamSize = std::strtoull(json_string_value(_mdJSONStreamSize), NULL, STARCH_RADIX);
                        _mdJSONStreamLineCount = json_object_get(_mdJSONStream, STARCH_METADATA_STREAM_LINECOUNT_KEY);
                        if (!_mdJSONStreamLineCount) {
                            if ((_archVersion->major >= 1) && (_archVersion->minor >= 3)) {
                                std::fprintf(stderr, "ERROR: Could not retrieve JSON stream's line count attribute\n");
                                std::fseek(_inFp, 0, SEEK_SET);
                                return false;
                            }
                            _mdJSONStreamLineCount = json_integer(STARCH_DEFAULT_LINE_COUNT);
                            _archStreamLineCount = (Bed::LineCountType) json_integer_value(_mdJSONStreamLineCount);
                            json_decref(_mdJSONStreamLineCount);
                        }
                        else 
                            _archStreamLineCount = (Bed::LineCountType) json_integer_value(_mdJSONStreamLineCount);

                        _mdJSONStreamNonUniqueBaseCount = json_object_get(_mdJSONStream, STARCH_METADATA_STREAM_TOTALNONUNIQUEBASES_KEY);
                        if (!_mdJSONStreamNonUniqueBaseCount) {
                            if ((_archVersion->major >= 1) && (_archVersion->minor >= 4)) {
                                std::fprintf(stderr, "ERROR: Could not retrieve JSON stream's non-unique base count attribute\n");
                                std::fseek(_inFp, 0, SEEK_SET);
                                return false;
                            }
                            _mdJSONStreamNonUniqueBaseCount = json_integer(STARCH_DEFAULT_NON_UNIQUE_BASE_COUNT);
                            _archStreamNonUniqueBaseCount = (Bed::BaseCountType) json_integer_value(_mdJSONStreamNonUniqueBaseCount);
                            json_decref(_mdJSONStreamNonUniqueBaseCount);
                        }
                        else 
                            _archStreamNonUniqueBaseCount = (Bed::BaseCountType) json_integer_value(_mdJSONStreamNonUniqueBaseCount);

                        _mdJSONStreamUniqueBaseCount = json_object_get(_mdJSONStream, STARCH_METADATA_STREAM_TOTALUNIQUEBASES_KEY);
                        if (!_mdJSONStreamUniqueBaseCount) {
                            if ((_archVersion->major >= 1) && (_archVersion->minor >= 4)) {
                                std::fprintf(stderr, "ERROR: Could not retrieve unique base count attribute\n");
                                std::fseek(_inFp, 0, SEEK_SET);
                                return false;
                            }
                            _mdJSONStreamUniqueBaseCount = json_integer(STARCH_DEFAULT_UNIQUE_BASE_COUNT);
                            _archStreamUniqueBaseCount = (Bed::BaseCountType) json_integer_value(_mdJSONStreamUniqueBaseCount);
                            json_decref(_mdJSONStreamUniqueBaseCount);
                        }
                        else 
                            _archStreamUniqueBaseCount = (Bed::BaseCountType) json_integer_value(_mdJSONStreamUniqueBaseCount);

                        if (_recIdx == 0) {
                            _testMd = STARCH_createMetadata(_archStreamChr,
                                                            _archStreamFn,
                                                            _archStreamSize,
                                                            _archStreamLineCount,
                                                            _archStreamNonUniqueBaseCount,
                                                            _archStreamUniqueBaseCount);
                            _firstMd = _testMd;
                        }
                        else
                            _testMd = STARCH_addMetadata(_testMd,
                                                         _archStreamChr,
                                                         _archStreamFn,
                                                         _archStreamSize,
                                                         _archStreamLineCount,
                                                         _archStreamNonUniqueBaseCount,
                                                         _archStreamUniqueBaseCount);
                        _recIdx++;
                        _mdJSONStreamChr = NULL;
                        _mdJSONStreamFn = NULL;
                        _mdJSONStreamSize = NULL;
                        _mdJSONStreamLineCount = NULL;
                        _mdJSONStreamNonUniqueBaseCount = NULL;
                        _mdJSONStreamUniqueBaseCount = NULL;
                        _mdJSONStream = NULL;
                    }
                }
                else {
                    std::fprintf(stderr, "ERROR: Archive version information not within acceptable range\n");
                    std::fseek(_inFp, 0, SEEK_SET);
                    return false;
                }
                free(_archVersion), _archVersion = NULL;
                free(_archStreamChr), _archStreamChr = NULL;
                free(_archStreamFn), _archStreamFn = NULL;
                json_decref(_mdJSON);

                _testMd = _firstMd;

                if (_testMd)
                    STARCH_freeMetadata(&_testMd);
                else {
                    std::fseek(_inFp, 0, SEEK_SET);
                    return false;
                }

                if (std::fseek(_inFp, 0, SEEK_SET) != 0) 
                    return false;

                return true;
            }

            static bool hasStarchRevision1Header(std::string& _inFn)
            {
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::hasStarchRevision1Header(std::string) ---\n");
#endif
                FILE *_testFp = NULL;

                _testFp = std::fopen(_inFn.c_str(), "r");
                if (!_testFp) 
                    return false;

                if (!hasStarchRevision1Header(_testFp)) {
                    std::fclose(_testFp);
                    return false;
                }

                if (std::fclose(_testFp) != 0)
                    return false;

                return true;
            }

            static bool hasStarchRevision1LegacyHeader(FILE *_inFp)
            {
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::hasStarchRevision1LegacyHeader(FILE *) ---\n");
#endif
                Metadata *_testMd = NULL;
                Metadata *_firstMd = NULL;
                size_t _testIdx, _bufIdx, _tokBufIdx, _tokCount, _recTokBufIdx;
                int _buf[STARCH_LEGACY_METADATA_SIZE] = {0};
                char _tokBuf[STARCH_LEGACY_METADATA_SIZE];
                char _recTokBuf[STARCH_LEGACY_METADATA_SIZE];
                int _recIdx = 0;
                char *_token = NULL;
                char *_tokenCheck = NULL;
                char *_recChromosome = NULL;
                char *_recFilename = NULL;
                uint64_t _recFileSize = UINT64_C(0);
                Bed::LineCountType _recLineCountValue = UINT64_C(0);
                Bed::BaseCountType _recNonUniqueBaseCountValue = UINT64_C(0);
                Bed::BaseCountType _recUniqueBaseCountValue = UINT64_C(0);

                /* read first 8 kilobytes into buffer */

                for (_testIdx = 0; _testIdx < STARCH_LEGACY_METADATA_SIZE; _testIdx++) {
                    _buf[_testIdx] = fgetc(_inFp);
                    if (_buf[_testIdx] == EOF)
                        return false;
                }

                /* parse buffer into Metadata records, if possible */

                for (_bufIdx = 0, _tokBufIdx = 0; _bufIdx < STARCH_LEGACY_METADATA_SIZE; _bufIdx++, _tokBufIdx++) {
                    if (_buf[_bufIdx] == '\n') {
                        /* parse line into record, if possible */
                        if (_tokBufIdx > 0)
                            _recIdx++;
                        _tokBuf[_tokBufIdx] = '\0';
                        _token = std::strtok(_tokBuf, "\t");
                        for (_tokCount = 0; _token != NULL; _tokCount++) {
                            switch (_tokCount) {
                                case 0: {
                                    _tokenCheck = STARCH_strnstr((const char *)_token, STARCH_LEGACY_EXTENSION_BZ2, strlen(_token));
                                    if (!_tokenCheck) {
                                        _tokenCheck = STARCH_strnstr((const char *)_token, STARCH_LEGACY_EXTENSION_GZIP, strlen(_token));
                                        if (!_tokenCheck)
                                            return false;
                                    }

                                    std::strncpy(_recTokBuf, _token, strlen(_token) + 1);
                                    for (_recTokBufIdx = 0; _recTokBufIdx < strlen(_recTokBuf); _recTokBufIdx++) {
                                        if (_recTokBuf[_recTokBufIdx] == '.') {
                                            _recTokBuf[_recTokBufIdx] = '\0';
                                            break;
                                        }
                                    }
                                    _recChromosome = (char *) std::malloc(std::strlen(_recTokBuf) + 1);
                                    if (!_recChromosome)
                                        return false;
                                    std::strncpy(_recChromosome, _recTokBuf, std::strlen(_recTokBuf) + 1);

                                    _recFilename = (char *) std::malloc(std::strlen(_token) + 1);
                                    if (!_recFilename)
                                        return false;
                                    std::strncpy(_recFilename, _token, strlen(_token) + 1);

                                    break;
                                }
                                case 1: {
                                    _recFileSize = std::strtoull(_token, NULL, STARCH_RADIX);
                                    break;
                                }
                            }
                            _token = std::strtok(NULL, "\t");
                        }

                        if (_tokBufIdx != 0) {
                            if (_recIdx == 1) {
                                _testMd = STARCH_createMetadata(_recChromosome, 
                                                                _recFilename, 
                                                                _recFileSize, 
                                                                _recLineCountValue,
                                                                _recNonUniqueBaseCountValue,
                                                                _recUniqueBaseCountValue);
                                _firstMd = _testMd;
                            }
                            else
                                _testMd = STARCH_addMetadata(_testMd, 
                                                             _recChromosome, 
                                                             _recFilename, 
                                                             _recFileSize, 
                                                             _recLineCountValue,
                                                             _recNonUniqueBaseCountValue,
                                                             _recUniqueBaseCountValue);
                        }
                        else
                            break;

                        _tokBufIdx = -1;
                    }
                    else
                        _tokBuf[_tokBufIdx] = (char) _buf[_bufIdx];
                }
    
                /* if _testMd is not NULL, then we have valid legacy metadata record(s) */

                if (std::fseek(_inFp, 0, SEEK_SET) != 0)
                    return false;

                _testMd = _firstMd;

                if (_testMd) {
                    STARCH_freeMetadata(&_testMd);
                    return true;
                }
    
                return false;
            }

            static bool hasStarchRevision1LegacyHeader(std::string& _inFn)
            {
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::hasStarchRevision1LegacyHeader(std::string) ---\n");
#endif
                FILE *_testFp = NULL;

                _testFp = std::fopen(_inFn.c_str(), "r");
                if (!_testFp) 
                    return false;

                if (!hasStarchRevision1LegacyHeader(_testFp)) {
                    std::fclose(_testFp);
                    return false;
                }

                if (std::fclose(_testFp) != 0)
                    return false;

                return true;
            }

            static bool isStarch(FILE *_inFp)
            {
                /* 
                    This tests if the file pointer contains a Starch archive
                    by testing if the file contains Starch-flavored markers.

                    Whether this test passes or fails, the FILE* will be kept 
                    open (if already open) when calling isStarch() directly
                    on a FILE*.

                    NOTE: The FILE* position will be reset to the first byte
                    upon conclusion of this test, pass or fail.
                */
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::isStarch(FILE *) ---\n");
#endif
                bool _rtn = false;
                if (!fpIsOpen(_inFp)) 
                    _rtn = false;
                else if (hasStarchRevision2Header(_inFp)) {
                    return true;
                }
                else if (hasStarchRevision1Header(_inFp)) {
                    return true;
                }
                else if (hasStarchRevision1LegacyHeader(_inFp)) {
                    return true;
                }

                std::rewind(_inFp);
                return _rtn;
            }

            static bool isStarch(std::string& _inFn) 
            {  
                /*
                    This tests if the file at the provided path contains a
                    Starch archive. If the file contains valid Starch-flavored 
                    markers, this function returns true. Otherwise, false.

                    NOTE: The file is closed at the conclusion of this test, 
                    pass or fail.
                */
#ifdef DEBUG
                std::fprintf(stderr, "\n--- Starch::isStarch(std::string) ---\n");
#endif
                if (!fnExists(_inFn)) return false;
                else if (hasStarchRevision2Header(_inFn)) {
                    return true;
                }
                else if (hasStarchRevision1Header(_inFn)) {
                    return true;
                }
                else if (hasStarchRevision1LegacyHeader(_inFn)) { 
                    return true;
                }

                return false;
            }

            std::string                     getInFn() { return inFn; }
            const char                    * getInFnCStr() { return inFn.c_str(); }
            Boolean                         getArchiveHeaderFlag() { return archHeaderFlag; }
            Boolean                       * getArchiveHeaderFlagPtr() { return &archHeaderFlag; }
            Boolean                         getArchiveShowNewlineFlag() { return archShowNewlineFlag; }
            Boolean                       * getArchiveShowNewlineFlagPtr() { return &archShowNewlineFlag; }
            Metadata                      * getArchiveMdIter() { return archMdIter; }
            Metadata                      * getArchiveRecordIter() { return archMdIter; }
            unsigned int                    getArchiveVersionMajor() { if (!archMd) { readJSONMetadata(false, false); } if (!archVersion) { throw(std::string("ERROR: Could not retrieve version data")); } return (unsigned int) archVersion->major; }
            unsigned int                    getArchiveVersionMinor() { if (!archMd) { readJSONMetadata(false, false); } if (!archVersion) { throw(std::string("ERROR: Could not retrieve version data")); } return (unsigned int) archVersion->minor; }
            unsigned int                    getArchiveVersionRevision() { if (!archMd) { readJSONMetadata(false, false); } if (!archVersion) { throw(std::string("ERROR: Could not retrieve version data")); } return (unsigned int) archVersion->revision; }
            char                          * getArchiveCreationTimestamp() { return archCreationTimestamp; }
            char                          * getArchiveNote() { return archNote; }
            inline const char             * getCurrentChromosome() { if ((!currentChromosome) && (archMdIter)) { setCurrentChromosome(archMdIter->chromosome); } return currentChromosome; }
            inline Bed::SignedCoordType     getCurrentStart() { return currentStart; }
            inline Bed::SignedCoordType     getCurrentStop() { return currentStop; }
            inline const char *             getCurrentRemainder() { return currentRemainder; }
            Bed::LineCountType              getCurrentChromosomeLineCount() { return (archMdIter) ? archMdIter->lineCount : UINT64_C(0); }
            Bed::BaseCountType              getCurrentChromosomeNonUniqueBaseCount() { return (archMdIter) ? archMdIter->totalNonUniqueBases : UINT64_C(0); }
            Bed::BaseCountType              getCurrentChromosomeUniqueBaseCount() { return (archMdIter) ? archMdIter->totalUniqueBases : UINT64_C(0); }
            inline bool                     isEOF() { return (!getCurrentChromosome()); }

        // ------------        
        private:
            Metadata *archMd;
            ArchiveVersion *archVersion;
            CompressionType archType;
            char *archCreationTimestamp;
            char *archNote;
            uint64_t archMdOffset;
            uint64_t archStreamOffset;
            Boolean archHeaderFlag;
            Boolean archShowNewlineFlag;
            json_t *archMdJSON;
            Metadata *archMdIter;
            FILE *inFp;
            std::string inFn;
            std::string selectedChromosome;
            uint64_t cumulativeSize;
            BZFILE *bzFp;
            unsigned char *bzOutput;
            z_stream zStream;
            char *zOutput;
            char *zRemainderBuf;
            char *zInBuf;
            char *zOutBuf;
            char *zLineBuf;
            int zError;
            int zBufIdx;
            int zOutBufIdx;
            int zHave;
            int zBufOffset;
            bool allowHeadersFlag;
            bool perLineUsageFlag;            
            char *currentChromosome;
            Bed::SignedCoordType currentStart;
            Bed::SignedCoordType currentStop;
            char *currentRemainder;
            Bed::SignedCoordType t_start;
            Bed::SignedCoordType t_pLength;
            Bed::SignedCoordType t_lastEnd;
            char t_firstInputToken[UNSTARCH_FIRST_TOKEN_MAX_LENGTH];
            char t_secondInputToken[UNSTARCH_SECOND_TOKEN_MAX_LENGTH];
            char *_currChr;
            size_t _currChrLen;
            Bed::SignedCoordType _currStart;
            Bed::SignedCoordType _currStop;
            char *_currRemainder;
            size_t _currRemainderLen;

            int initializeMembers();
            int setupBzip2Works();
            int breakdownBzip2Works();
            int setupGzipWorks();
            int breakdownGzipWorks();
            int setupTransformationParameters();
            int seekCurrentInFpPosition();
            int zReadChunk();
            int zReadLine();
            int extractLine(std::string& line);
            int setupPerLineAccess();
            int readJSONMetadata(bool suppressErrorMsgs, bool preserveJSONRef);

            Metadata * getArchiveMd() { return archMd; }
            Metadata ** getArchiveMdPtr() { return &archMd; }
            ArchiveVersion * getArchiveVersion() { return archVersion; }
            ArchiveVersion ** getArchiveVersionPtr() { return &archVersion; }
            CompressionType getArchiveType() { return archType; }
            CompressionType * getArchiveTypePtr() { return &archType; }
            char ** getArchiveCreationTimestampPtr() { return &archCreationTimestamp; }
            char ** getArchiveNotePtr() { return &archNote; }
            uint64_t getArchiveMdOffset() { return archMdOffset; }
            uint64_t * getArchiveMdOffsetPtr() { return &archMdOffset; }
            uint64_t getArchiveStreamOffset() { return archStreamOffset; }
            uint64_t * getArchiveStreamOffsetPtr() { return &archStreamOffset; }
            json_t * getArchiveMdJSON() { return archMdJSON; }
            json_t ** getArchiveMdJSONPtr() { return &archMdJSON; }
            FILE * getInFp() { return inFp; }
            FILE ** getInFpPtr() { return &inFp; }
            BZFILE * getBzFp() { return bzFp; }
            BZFILE ** getBzFpPtr() { return &bzFp; }
            unsigned char * getBzOutput() { return bzOutput; }
            unsigned char ** getBzOutputPtr() { return &bzOutput; }
            z_stream getZStream() { return zStream; }
            z_stream * getZStreamPtr() { return &zStream; }
            char * getZOutput() { return zOutput; }
            char ** getZOutputPtr() { return &zOutput; }
            char * getZRemainderBuf() { return zRemainderBuf; }
            char ** getZRemainderBufPtr() { return &zRemainderBuf; }
            char * getZInBuf() { return zInBuf; }
            char ** getZInBufPtr() { return &zInBuf; }
            char * getZOutBuf() { return zOutBuf; }
            char ** getZOutBufPtr() { return &zOutBuf; }
            char * getZLineBuf() { return zLineBuf; }
            char ** getZLineBufPtr() { return &zLineBuf; }
            int getZError() { return zError; }
            int getZBufIdx() { return zBufIdx; }
            int getZOutBufIdx() { return zOutBufIdx; }
            int getZHave() { return zHave; }
            int getZBufOffset() { return zBufOffset; }            
            bool getAllowHeadersFlag() { return allowHeadersFlag; }
            bool getPerLineUsageFlag() { return perLineUsageFlag; }
            uint64_t getCumulativeSize() { return cumulativeSize; }
            void setInFn(const std::string& _inFn) { inFn = _inFn; }
            void setArchiveHeaderFlag(unsigned int _aHf) { archHeaderFlag = _aHf; }
            void setArchiveShowNewlineFlag(unsigned int _aSNf) { archShowNewlineFlag = _aSNf; }
            void setArchiveMd(Metadata *_aMd) { archMd = _aMd; }
            void setArchiveVersion(ArchiveVersion *_av) { archVersion = _av; }
            void setArchiveType(CompressionType _type) { archType = _type; }
            void setArchiveCreationTimestamp(const char *_cTime) { archCreationTimestamp = (char *) _cTime; }
            void setArchiveNote(const char *_note) { archNote = (char *) _note; }
            void setArchiveMdOffset(uint64_t _aMdOffset) { archMdOffset = _aMdOffset; }
            void setArchiveStreamOffset(uint64_t _aStreamOffset) { archStreamOffset = _aStreamOffset; }
            void setArchiveMdIter(Metadata *_aMdIter) { archMdIter = _aMdIter; }
            void setArchiveMdJSON(json_t *_aMdJSON) { archMdJSON = _aMdJSON; }
            void incrementCumulativeSize(uint64_t _inc) { cumulativeSize += _inc; }            
            void setBzOutput(unsigned char * _bzOutput) { bzOutput = _bzOutput; }
            void setZOutput(char * _zOutput) { zOutput = _zOutput; }
            void setZInBuf(char * _zInBuf) { zInBuf = _zInBuf; }
            void setZOutBuf(char * _zOutBuf) { zOutBuf = _zOutBuf; }
            void setZLineBuf(char * _zLineBuf) { zLineBuf = _zLineBuf; }
            void setZError(int _zError) { zError = _zError; }
            void setZBufIdx(int _zBufIdx) { zBufIdx = _zBufIdx; }
            void setZOutBufIdx(int _zOutBufIdx) { zOutBufIdx = _zOutBufIdx; }
            void setZHave(int _zHave) { zHave = _zHave; }
            void setZBufOffset(int _zBufOffset) { zBufOffset = _zBufOffset; }
            void setAllowHeadersFlag(bool _allowHeadersFlag) { allowHeadersFlag = _allowHeadersFlag; }
            void setPerLineUsageFlag(bool _perLineUsageFlag) { perLineUsageFlag = _perLineUsageFlag; }
            void iterateArchiveMdIter() {
              if (!archMdIter)
                return;
              archMdIter = archMdIter->next;
              if (archMdIter)
                setCurrentChromosome(archMdIter->chromosome);
              else
                free(currentChromosome), currentChromosome = NULL;
            }
            void setCurrentChromosome(char *_cC) {
              if (currentChromosome) {
                free(currentChromosome);
                currentChromosome = NULL;
              }
              currentChromosome = (char *) malloc (strlen(_cC) + 1);
              if (currentChromosome) {
                strncpy(currentChromosome, _cC, strlen(_cC) + 1);
              }
            }
            void setCurrentStart(Bed::SignedCoordType  _cS) { currentStart = _cS; }
            void setCurrentStop(Bed::SignedCoordType _cS) { currentStop = _cS; }
            void setCurrentRemainder(char * _remainder) { currentRemainder = _remainder; }
            void setSelectedChromosome(const std::string& _selChr) { selectedChromosome = _selChr; }
    };

    Starch::Starch() 
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::Starch() ---\n");
#endif
        initializeMembers();

        if (perLineUsageFlag)
            setupPerLineAccess();
    }

    Starch::Starch(FILE *_inFP, 
      const std::string& _chr, 
                    bool _perLineUsage)
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::Starch(FILE *, std::string, bool) ---\n");
#endif
        initializeMembers();

        inFn = "unknown";
        inFp = _inFP;
        selectedChromosome = _chr;
        perLineUsageFlag = _perLineUsage;
        if (perLineUsageFlag)
            setupPerLineAccess();
    }

    Starch::Starch(FILE *_inFp, 
      const std::string& _inFn, 
      const std::string& _chr, 
                    bool _perLineUsage)
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::Starch(FILE *, std::string, std::string, bool) ---\n");
#endif
        initializeMembers();

        inFn = _inFn;
        inFp = _inFp;
        selectedChromosome = _chr;
        perLineUsageFlag = _perLineUsage;

        if (perLineUsageFlag)
            setupPerLineAccess();
    }

    Starch::Starch(const std::string& _inFn)
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::Starch(std::string) ---\n");
#endif
        initializeMembers();

        inFn = _inFn;

        if (perLineUsageFlag)
            setupPerLineAccess();
    }

    Starch::Starch(const std::string &_inFn, 
                          const bool _allowHeadersFlag) 
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::Starch(std::string, bool) ---\n");
#endif
        initializeMembers();

        inFn = _inFn;
        allowHeadersFlag = _allowHeadersFlag;

        if (perLineUsageFlag)
            setupPerLineAccess();
    }

    Starch::Starch(const std::string &_inFn, 
                          const bool _allowHeadersFlag, 
                          const bool _perLineUsageFlag, 
                   const std::string &_selectedChromosome) 
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::Starch(std::string, bool, bool, std::string) ---\n");
#endif
        initializeMembers();

        inFn = _inFn;
        allowHeadersFlag = _allowHeadersFlag;
        perLineUsageFlag = _perLineUsageFlag;
        selectedChromosome = _selectedChromosome;

        if (perLineUsageFlag)
            setupPerLineAccess();
    }

    Starch::Starch(const std::string& _inFn, 
                           const bool _allowHeadersFlag,
                           const bool _perLineUsageFlag,
                           Metadata * _md, 
                      CompressionType _type, 
                     ArchiveVersion * _av, 
                             uint64_t _mdOffset,
                         unsigned int _hf) 
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::Starch(std::string, bool, bool, Metadata *, CompressionType, ArchiveVersion *, uint64_t, unsigned int) ---\n");
#endif
        initializeMembers();

        inFn = _inFn;
        allowHeadersFlag = _allowHeadersFlag;
        perLineUsageFlag = _perLineUsageFlag;
        archMd = _md;
        archType = _type;
        archVersion = _av;
        archMdOffset = _mdOffset;
        archHeaderFlag = _hf;
        
        if (!inFn.empty()) {
            inFp = std::fopen(inFn.c_str(), "rbR");
            if (!inFp) {
                throw("ERROR: could not open handle to " + inFn);
                exit (EXIT_FAILURE);
            }
        }

        if (perLineUsageFlag)
            setupPerLineAccess();
    }    

    Starch::~Starch() 
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::~Starch() ---\n");
#endif
        if (archMd != NULL) 
            STARCH_freeMetadata(&archMd), archMd = NULL;
        if (archVersion != NULL) 
            delete archVersion, archVersion = NULL;
        if (inFp != NULL) 
            std::fclose(inFp), inFp = NULL;
        if (bzOutput != NULL)
            free(bzOutput), bzOutput = NULL;
    }

    Starch::Starch(const Starch& cpArchive) 
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::Starch(Starch &) ---\n");
#endif
        if (cpArchive.inFn.empty()) { inFn.assign(cpArchive.inFn); } else { inFn = std::string(); }
        if (cpArchive.archMd != NULL) { archMd = STARCH_copyMetadata((const Metadata *)cpArchive.archMd); } else { archMd = NULL; }
        if (cpArchive.archVersion != NULL) {
            if (!archVersion)
                archVersion = new ArchiveVersion;
            if (!archVersion)
                throw(std::string("ERROR: could not allocate space for archive version struct"));
            archVersion->major = cpArchive.archVersion->major;
            archVersion->minor = cpArchive.archVersion->minor;
            archVersion->revision = cpArchive.archVersion->revision;
        }
        archType = cpArchive.archType;
        archMdOffset = cpArchive.archMdOffset;
        archHeaderFlag = cpArchive.archHeaderFlag;
        archShowNewlineFlag = cpArchive.archShowNewlineFlag;

        if (!inFn.empty()) { 
            inFp = std::fopen(inFn.c_str(), "rbR");
            if (!inFp)
                throw("ERROR: could not open handle to " + inFn);
        }
    }

    Starch& 
    Starch::operator=(const Starch& cpArchive) 
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::operator=(Starch &) ---\n");
#endif
        if (this == &cpArchive)
            return *this;

        if (archMd != NULL) STARCH_freeMetadata(&archMd);
        if (archVersion != NULL) free(archVersion);

        if (cpArchive.inFn.empty()) { inFn.assign(cpArchive.inFn); } else { inFn = std::string(); }
        if (cpArchive.archMd != NULL) { archMd = STARCH_copyMetadata((const Metadata *)cpArchive.archMd); } else { archMd = NULL; }
        if (cpArchive.archVersion != NULL) { 
            archVersion = new ArchiveVersion;
            if (!archVersion)
                throw(std::string("ERROR: could not allocate space for archive version struct"));
            archVersion->major = cpArchive.archVersion->major;
            archVersion->minor = cpArchive.archVersion->minor;
            archVersion->revision = cpArchive.archVersion->revision;
        }
        archType = cpArchive.archType;
        archMdOffset = cpArchive.archMdOffset;
        archHeaderFlag = cpArchive.archHeaderFlag;
        archShowNewlineFlag = cpArchive.archShowNewlineFlag;

        if (!inFn.empty()) { 
            inFp = std::fopen(inFn.c_str(), "rbR");
            if (!inFp)
                throw("ERROR: could not open handle to " + inFn);
        }

        return *this;
    }

    int 
    Starch::initializeMembers()
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::initializeMembers() ---\n");
#endif
        selectedChromosome = "";
        archMd = NULL;
        archVersion = NULL;
        archType = STARCH_DEFAULT_COMPRESSION_TYPE;
        archCreationTimestamp = NULL;
        archNote = NULL;
        archMdOffset = 0ULL;
        archStreamOffset = 0ULL;
        archHeaderFlag = kStarchFalse;
        archShowNewlineFlag = kStarchTrue;
        archMdIter = NULL;
        inFp = NULL;
        inFn = "";
        cumulativeSize = UINT64_C(0);
        bzFp = NULL;
        bzOutput = NULL;
        zOutput = NULL;
        zRemainderBuf = NULL;
        zInBuf = NULL;
        zOutBuf = NULL;
        zLineBuf = NULL;
        zError = Z_OK;
        zBufIdx = 0;
        zOutBufIdx = 0;
        zHave = 0;
        zBufOffset = 0;
        allowHeadersFlag = false;
        perLineUsageFlag = false;
        currentChromosome = NULL;
        currentStart = -2LL;
        currentStop = INT64_C(0);
        currentRemainder = NULL;
        t_start = INT64_C(0);
        t_pLength = INT64_C(0);
        t_lastEnd = INT64_C(0);
        t_firstInputToken[0] = '\0';
        t_secondInputToken[0] = '\0';
        archMdJSON = NULL;
        _currChr = NULL;
        _currChrLen = 0;
        _currStart = INT64_C(0);
        _currStop = INT64_C(0);
        _currRemainder = NULL;
        _currRemainderLen = 0;

        archVersion = new ArchiveVersion;
        if (!archVersion)
            throw(std::string("ERROR: could not allocate space for archive version struct"));
        archVersion->major = -1;
        archVersion->minor = -1;
        archVersion->revision = -1;

        return EXIT_SUCCESS;
    }

    int 
    Starch::readJSONMetadata(bool _suppressErrorMsgs, bool _preserveJSONRef) 
    {   
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::readJSONMetadata(bool, bool) ---\n");
#endif
        const Boolean suppressErrorMsgs = (_suppressErrorMsgs) ? kStarchTrue : kStarchFalse;
        const Boolean preserveJSONRef = (_preserveJSONRef) ? kStarchTrue : kStarchFalse;
        const int result = STARCH_readJSONMetadata(getArchiveMdJSONPtr(),
                                       getInFpPtr(), 
                                       getInFn().c_str(), 
                                       getArchiveMdPtr(), 
                                       getArchiveTypePtr(),                                     
                                       getArchiveVersionPtr(),
                                       getArchiveCreationTimestampPtr(),
                                       getArchiveNotePtr(),
                                       getArchiveMdOffsetPtr(),
                                       getArchiveHeaderFlagPtr(),
                                       suppressErrorMsgs,
                                       preserveJSONRef);

        switch (archVersion->major) {
            case 1: {
                archStreamOffset = archMdOffset;
                break;
            }
            case 2: {
                cumulativeSize = 0ULL;
                archStreamOffset = STARCH2_MD_HEADER_BYTE_LENGTH;
                break;
            }
            default:
                throw(std::string("ERROR: Could not determine stream offset from unknown major version"));
        }

        return result;
    }

    int
    Starch::listJSONMetadata(FILE *out, 
                             FILE *err) 
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::listJSONMetadata(FILE *, FILE *) ---\n");
#endif
        if (!archMd) 
            readJSONMetadata(false, false);

        return STARCH_listJSONMetadata(out,
                                       err,
                                       (const Metadata *) getArchiveMd(),
                                       (const CompressionType) getArchiveType(),
                                       (const ArchiveVersion *) getArchiveVersion(),
                                       (const char *) getArchiveCreationTimestamp(),
                                       (const char *) getArchiveNote(),
                                       (const Boolean) getArchiveHeaderFlag(),
                                       (const Boolean) getArchiveShowNewlineFlag());
    }

    int
    Starch::setupBzip2Works()
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::setupBzip2Works() ---\n");
#endif
        int bzError;
        size_t bzOutputLength = UNSTARCH_COMPRESSED_BUFFER_MAX_LENGTH;

        if (!bzFp) 
        {
            // opening bzip2 handle...
            // http://www.bzip.org/1.0.5/bzip2-manual-1.0.5.html#bzcompress-init
            bzFp = BZ2_bzReadOpen( &bzError, getInFp(), 0, 0, NULL, 0 ); 
            if (bzError != BZ_OK) {
                BZ2_bzReadClose( &bzError, bzFp );
                throw(std::string("ERROR: bzip2 data stream could not be opened"));
            }

            // setting up initial bz output buffer...
            if (!bzOutput) 
                bzOutput = (unsigned char *) std::malloc(bzOutputLength);
            if (!bzOutput)
                throw(std::string("ERROR: bzip2 output buffer could not be allocated"));

            // seting up transformation parameters...
            if (setupTransformationParameters() != EXIT_SUCCESS)
                throw(std::string("ERROR: could not initialize transformation parameters"));

            // fseek to correct offset...
            //if (seekCurrentInFpPosition() != EXIT_SUCCESS)
            //    throw(std::string("ERROR: could not jump to offset"));
        }
        else
            throw(std::string("ERROR: bzip2 data stream is already open"));

        return EXIT_SUCCESS;
    }

    int
    Starch::breakdownBzip2Works()
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::breakdownBzip2Works() ---\n");
#endif
        int bzError = BZ_STREAM_END;

        if (bzOutput)
            free(bzOutput), bzOutput = NULL;

        if (bzFp)
            BZ2_bzReadClose( &bzError, bzFp ), bzFp = NULL;

        switch (bzError) {
            case (BZ_SEQUENCE_ERROR): {
                throw(std::string("ERROR: bzFp was opened with BZ2_bzOpenWrite"));
            }
            default:
                break;
        }

        return EXIT_SUCCESS;
    }

    int
    Starch::setupGzipWorks()
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::setupGzipWorks() ---\n");
#endif
        zStream.zalloc = Z_NULL;
        zStream.zfree = Z_NULL;
        zStream.opaque = Z_NULL;
        zStream.avail_in = 0;
        zStream.avail_out = 0;
        zStream.next_in = Z_NULL;

        zError = inflateInit2(&zStream, (15+32)); /* cf. http://www.zlib.net/manual.html */
        switch (zError) {
            case Z_MEM_ERROR: {
                throw(std::string("ERROR: ran out of memory to initialize z-stream"));
            }
            case Z_VERSION_ERROR: {
                throw(std::string("ERROR: zlib library version incompatible with z-stream"));
            }
            case Z_STREAM_ERROR: {
                throw(std::string("ERROR: invalid parameters for setting up z-stream"));
            }
            default:
                break;
        }

        zRemainderBuf = (char *) std::malloc(1);
        if (!zRemainderBuf)
            throw(std::string("ERROR: ran out of memory to allocate to z-remainder-buffer"));
        zRemainderBuf[0] = '\0';

        zInBuf = (char *) std::malloc(STARCH_Z_CHUNK);
        if (!zInBuf)
            throw(std::string("ERROR: ran out of memory to allocate to z-in-buffer"));
        zInBuf[0] = '\0';

        zOutBuf = (char *) std::malloc(STARCH_Z_CHUNK);
        if (!zOutBuf)
            throw(std::string("ERROR: ran out of memory to allocate to z-in-buffer"));
        zOutBuf[0] = '\0';

        zLineBuf = (char *) std::malloc(STARCH_Z_CHUNK);
        if (!zLineBuf)
            throw(std::string("ERROR: ran out of memory to allocate to z-in-buffer"));
        zLineBuf[0] = '\0';

        // seting up transformation parameters...
        if (setupTransformationParameters() != EXIT_SUCCESS)
            throw(std::string("ERROR: could not initialize transformation parameters"));
        return EXIT_SUCCESS;
    }

    int
    Starch::breakdownGzipWorks()
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::breakdownGzipWorks() ---\n");
#endif
        zError = inflateEnd(&zStream);
        switch (zError) {
            case Z_STREAM_ERROR: {
                throw(std::string("ERROR: z-stream state inconsistent"));
            }
            case Z_DATA_ERROR: {
                throw(std::string("ERROR: z-stream freed prematurely"));
            }
            default:
                break;
        }

        if (zRemainderBuf)
            free(zRemainderBuf), zRemainderBuf = NULL;
        if (zInBuf)
            free(zInBuf), zInBuf = NULL;
        if (zOutBuf)
            free(zOutBuf), zOutBuf = NULL;
        if (zLineBuf)
            free(zLineBuf), zLineBuf = NULL;

        return EXIT_SUCCESS;
    }

    int
    Starch::setupTransformationParameters()
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::setupTransformationParameters() ---\n");
        std::fprintf(stderr, "\tarchVersion->major -> %d\n", archVersion->major);
#endif
        t_start = INT64_C(0);
        t_pLength = INT64_C(0);
        t_lastEnd = INT64_C(0);
        t_firstInputToken[0] = '\0';
        t_secondInputToken[0] = '\0';

        /*
            We check the archive version and set the stream offset parameters
            to initialize transformation
        */

        //cumulativeSize = 0ULL;
        switch (archVersion->major) {
            case 1: {
                archStreamOffset = archMdOffset;
                break;
            }
            case 2: {
                archStreamOffset = STARCH2_MD_HEADER_BYTE_LENGTH;
                break;
            }
            default:
                throw(std::string("ERROR: Could not determine stream offset from unknown major version"));
        }
#ifdef DEBUG
        std::fprintf(stderr, "\tarchStreamOffset -> %" PRIu64 "\n", archStreamOffset);
#endif
        
        return EXIT_SUCCESS;
    }

    int
    Starch::seekCurrentInFpPosition()
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::seekCurrentInFpPosition() ---\n");
        std::fprintf(stderr, "\tcumulative size -> %" PRId64 "\n", cumulativeSize);
        std::fprintf(stderr, "\tarchive stream offset -> %" PRId64 "\n", archStreamOffset);
        std::fprintf(stderr, "\toffsetting to %" PRIu64 "\n", (cumulativeSize + archStreamOffset));
#endif
        if (archMdIter) {
            if (STARCH_fseeko( getInFp(), (off_t)(cumulativeSize + archStreamOffset), SEEK_SET ) != 0)
                throw(std::string("ERROR: could not seek data in archive"));
            incrementCumulativeSize( archMdIter->size );
        }

        return EXIT_SUCCESS;
    }

    bool
    Starch::extractBEDLine(std::string& line)
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::extractBEDLine(std::string &) ---\n");
#endif
        line.clear();
        while (!isEOF()) {
            extractLine(line);

            if ((t_firstInputToken[0] == 'p') && (archMdIter))
                extractLine(line);

            if (!line.empty())
                break;
        }

        return !isEOF();
    }

    int
    Starch::extractLine(std::string& line)
    { 
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::extractLine(std::string &) ---\n");
#endif
        static char out[STARCH_BUFFER_MAX_LENGTH];
        static const char tab = '\t';
        int res = 0;

        if ((std::strcmp(selectedChromosome.c_str(), "all") == 0) || (std::strcmp(selectedChromosome.c_str(), getCurrentChromosome()) == 0)) {
            switch (archType) {
                case kBzip2: {
                    // extract untransformed line from archive
                    UNSTARCH_bzReadLine(bzFp, &bzOutput);
                    if (bzOutput) {
                        // we deliberately choose to disable support for headers 
                        // in reverse transformation used by C++ client apps
                        allowHeadersFlag = false;

                        // transform data back to BED
                        if (allowHeadersFlag) {
                            (archHeaderFlag == kStarchFalse) ?
                                UNSTARCH_sReverseTransformHeaderlessInput( getCurrentChromosome(),
                                                                           (const unsigned char *) bzOutput, 
                                                                           tab,
                                                                           &t_start, 
                                                                           &t_pLength, 
                                                                           &t_lastEnd,
                                                                           t_firstInputToken, 
                                                                           t_secondInputToken,
                                                                           &_currChr,
                                                                           &_currChrLen,
                                                                           &_currStart,
                                                                           &_currStop,
                                                                           &_currRemainder,
                                                                           &_currRemainderLen)
                                              :
                                UNSTARCH_sReverseTransformInput( getCurrentChromosome(),
                                                                 (const unsigned char *) bzOutput, 
                                                                 tab,
                                                                 &t_start, 
                                                                 &t_pLength, 
                                                                 &t_lastEnd,
                                                                 t_firstInputToken, 
                                                                 t_secondInputToken,
                                                                 &_currChr,
                                                                 &_currChrLen,
                                                                 &_currStart,
                                                                 &_currStop,
                                                                 &_currRemainder,
                                                                 &_currRemainderLen);
                        }
                        else {
                            do {
                                res = UNSTARCH_sReverseTransformIgnoringHeaderedInput( getCurrentChromosome(),
                                                                                       (const unsigned char *) bzOutput, 
                                                                                       tab,
                                                                                       &t_start, 
                                                                                       &t_pLength, 
                                                                                       &t_lastEnd,
                                                                                       t_firstInputToken, 
                                                                                       t_secondInputToken,
                                                                                       &_currChr,
                                                                                       &_currChrLen,
                                                                                       &_currStart,
                                                                                       &_currStop,
                                                                                       &_currRemainder,
                                                                                       &_currRemainderLen);
                                if (res != 0)
                                    extractLine(line);
                            } while (res != 0);
                        }

                        // if the first character of the first untransformed token is 'p', then
                        // we have not yet extracted a BED line, and so we call extractLine()
                        // once again to get BED output

#ifdef DEBUG
                        std::fprintf(stderr, "\t TOKENS --> t_firstInputToken: %s \t t_secondInputToken: %s\n", t_firstInputToken, t_secondInputToken);
#endif

                        if (t_firstInputToken[0] == 'p')
                            extractLine(line);

                        t_firstInputToken[0] = '\0';
                        t_secondInputToken[0] = '\0';
                    }
                    else {
                        // we break down bzip2-workings, then go to the next
                        // metadata record. if it is not NULL, then we seek the
                        // next byte offset and set up a new bzip2 reader. if the 
                        // metadata is NULL, then we are positioned at EOF and 
                        // we break.

                        breakdownBzip2Works();
                        if (std::strcmp(selectedChromosome.c_str(), getCurrentChromosome()) == 0) {
                            archMdIter = NULL;
                            if (currentChromosome) free(currentChromosome), currentChromosome = NULL;
                            if (_currChr) free(_currChr), _currChr = NULL;
                            if (_currRemainder) free(_currRemainder), _currRemainder = NULL;
                            line.clear();
                            return EXIT_SUCCESS;
                        }
                        iterateArchiveMdIter();
                        if (!archMdIter) {
                            archMdIter = NULL;
                            if (currentChromosome) free(currentChromosome), currentChromosome = NULL;
                            if (currentRemainder) free(currentRemainder), currentRemainder = NULL;
                            if (_currChr) free(_currChr), _currChr = NULL;
                            line.clear();
                            return EXIT_SUCCESS;
                        }
                        seekCurrentInFpPosition();
                        setupBzip2Works();

                        // we call extractLine() once more, in order to get the next
                        // BED element (we're not interested in untransformed data, but
                        // in a fully-transformed line of BED output)

                        extractLine(line);
                    }
                    break;
                }

                case kGzip: {
                    // extract untransformed line from archive
                    zReadLine();
                    if (zHave > 0) {
                        // we deliberately choose to disable support for headers 
                        // in reverse transformation used by C++ client apps
                        allowHeadersFlag = false;

                        // transform data back to BED, sending it to outbound FILE* 
                        if (allowHeadersFlag) {
                            (archHeaderFlag == kStarchFalse) ?
                                UNSTARCH_sReverseTransformHeaderlessInput( getCurrentChromosome(),
                                                                           (const unsigned char *) zLineBuf, 
                                                                           tab,
                                                                           &t_start, 
                                                                           &t_pLength, 
                                                                           &t_lastEnd,
                                                                           t_firstInputToken, 
                                                                           t_secondInputToken,
                                                                           &_currChr,
                                                                           &_currChrLen,
                                                                           &_currStart,
                                                                           &_currStop,
                                                                           &_currRemainder,
                                                                           &_currRemainderLen)
                                              :
                                UNSTARCH_sReverseTransformInput( getCurrentChromosome(),
                                                                 (const unsigned char *) zLineBuf, 
                                                                 tab,
                                                                 &t_start, 
                                                                 &t_pLength, 
                                                                 &t_lastEnd,
                                                                 t_firstInputToken, 
                                                                 t_secondInputToken,
                                                                 &_currChr,
                                                                 &_currChrLen,
                                                                 &_currStart,
                                                                 &_currStop,
                                                                 &_currRemainder,
                                                                 &_currRemainderLen);
                        }
                        else {
                            do {
                                res = UNSTARCH_sReverseTransformIgnoringHeaderedInput( getCurrentChromosome(),
                                                                                       (const unsigned char *) zLineBuf, 
                                                                                       tab,
                                                                                       &t_start, 
                                                                                       &t_pLength,
                                                                                       &t_lastEnd,
                                                                                       t_firstInputToken, 
                                                                                       t_secondInputToken,
                                                                                       &_currChr,
                                                                                       &_currChrLen,
                                                                                       &_currStart,
                                                                                       &_currStop,
                                                                                       &_currRemainder,
                                                                                       &_currRemainderLen);
                                if (res != 0)
                                    extractLine(line);
                            } while (res != 0);
                        }

                        // if the first character of the first untransformed token is 'p', then
                        // we have not yet extracted a BED line, and so we call extractLine()
                        // once again to get actual BED output

                        if (t_firstInputToken[0] == 'p')
                            extractLine(line);

                        t_firstInputToken[0] = '\0';
                        t_secondInputToken[0] = '\0';
                    }
                    else {
                        // we break down gzip-workings, then go to the next
                        // metadata record. if it is not NULL, then we seek the
                        // next byte offset and set up a new gzip reader. if the 
                        // metadata is NULL, then we are positioned at EOF and 
                        // we break.

                        breakdownGzipWorks();
                        if (std::strcmp(selectedChromosome.c_str(), getCurrentChromosome()) == 0) {
                            archMdIter = NULL;
                            if (currentChromosome) free(currentChromosome), currentChromosome = NULL;
                            if (_currChr) free(_currChr), _currChr = NULL;
                            if (_currRemainder) free(_currRemainder), _currRemainder = NULL;
                            line.clear();
                            return EXIT_SUCCESS;
                        }
                        iterateArchiveMdIter();
                        if (!archMdIter) {
                            archMdIter = NULL;
                            if (currentChromosome) free(currentChromosome), currentChromosome = NULL;
                            if (currentRemainder) free(currentRemainder), currentRemainder = NULL;
                            if (_currChr) free(_currChr), _currChr = NULL;
                            line.clear();
                            return EXIT_SUCCESS;
                        }
                        seekCurrentInFpPosition();
                        setupGzipWorks();

                        // we call extractLine() once more, in order to get the next
                        // BED element (we're not interested in untransformed data, but
                        // in a fully-transformed line of BED output)
            
                        extractLine(line);
                    }
                    break;
                }
                case kUndefined: {
                    throw(std::string("ERROR: backend compression type is undefined"));
                }
            }            
        }

        else {

            // if we are not in the desired chromosome, we don't do any parsing
            // but instead iterate the internal metadata iterator, seek the 
            // new offset, and then we look at the next chromosome on the next
            // run of extractLine() (if there is a next chromosome -- otherwise,
            // we would be at EOF and we are done parsing altogether)

            iterateArchiveMdIter();
            seekCurrentInFpPosition();
        }

        if (_currChr) {
            setCurrentStart(_currStart);
            setCurrentStop(_currStop);
            setCurrentRemainder(_currRemainder);
            if (_currRemainder)
                std::sprintf(out, "%s\t%" PRId64 "\t%" PRId64 "\t%s", _currChr, _currStart, _currStop, _currRemainder);
            else
                std::sprintf(out, "%s\t%" PRId64 "\t%" PRId64, _currChr, _currStart, _currStop);
            line = out;
        }

        return EXIT_SUCCESS;
    }

    int 
    Starch::zReadChunk()
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::zReadChunk() ---\n");
#endif
        zStream.avail_in = std::fread(zInBuf, 1, STARCH_Z_CHUNK, inFp);
        zStream.next_in = (Byte *) zInBuf;

        zStream.avail_out = STARCH_Z_CHUNK;
        zStream.next_out = (Byte *) zOutBuf;

        zError = inflate(&zStream, Z_NO_FLUSH);
        switch (zError) {
            case Z_NEED_DICT: {
                throw(std::string("ERROR: z-stream needs dictionary"));
            }
            case Z_DATA_ERROR: {
                throw(std::string("ERROR: z-stream suffered data error"));
            }
            case Z_MEM_ERROR: {
                throw(std::string("ERROR: z-stream suffered memory error"));
            }
            default:
                break;
        }
        zHave = STARCH_Z_CHUNK - zStream.avail_out;
        zOutBuf[zHave] = '\0';

        // copy remainder buffer onto the line buffer, if not NULL
        if (zRemainderBuf) {
            std::strncpy(zLineBuf, zRemainderBuf, strlen(zRemainderBuf));
            zBufOffset = std::strlen(zRemainderBuf);
        }
        else
            zBufOffset = 0;

        // set intial conditions for line parsing
        zBufIdx = zBufOffset;
        zOutBufIdx = 0;
        
        return EXIT_SUCCESS;
    }

    int
    Starch::zReadLine()
    {       
        // goal: read through zOutBuf until we hit a newline, then return with
        //       a filled line buffer containing transformed BED data
        //
        // -- we initialize zBufIdx and zOutBufIdx in zReadChunk()
        //
        //    ---- we don't (re)initialize them here, as we use them to
        //         maintain positional state between calls to zReadLine()
        //
        //    ---- if zOutBufIdx is zero, then we need to read in a
        //         z-chunk, in order to "prime the pump" and fill
        //         the buffers up with the first batch of data

#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::zReadLine() ---\n");
#endif

        if (zOutBufIdx == 0)
            zReadChunk();

        for (; zOutBufIdx < zHave; zBufIdx++) {
            if (zOutBuf[zOutBufIdx++] == '\n') {
                zLineBuf[zBufIdx] = '\0';
                zBufIdx = 0;
                return EXIT_SUCCESS;
            }
            else
                zLineBuf[zBufIdx] = zOutBuf[zOutBufIdx - 1];
        }

        // if we do not find a newline in a sweep through zOutBuf, then we 
        // will need to:
        //
        // -- put any remaining bytes into the remainder buffer (which
        //    are put back at the start of the line buffer on the next read
        //    of a z-chunk)
        //
        // -- read another z-chunk (if necessary; if we do read another
        //    z-chunk, then we are in the middle of a line and need to
        //    start parsing for newlines again)

        if (std::strlen(zLineBuf) > 0) {            
            if (std::strlen(zLineBuf) > std::strlen(zRemainderBuf)) {
                free(zRemainderBuf), zRemainderBuf = NULL;
                zRemainderBuf = (char *) std::malloc(std::strlen(zLineBuf) * 2); // resize remainder buffer, if necessary
            }
            std::strncpy(zRemainderBuf, zLineBuf, zBufIdx);
            zRemainderBuf[zBufIdx] = '\0';
        }

        if (zOutBufIdx == zHave) {
            if (zHave == 0)
                return EXIT_SUCCESS;
            else {
                zReadChunk();
                zReadLine();
            }
        }
        
        return EXIT_SUCCESS;
    }

    int 
    Starch::extractAllData(const std::string& chr, FILE *out)
    {
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::extractAllData(std::string &, FILE *) ---\n");
#endif
        if (!archMd) 
            readJSONMetadata(false, false);
        
        switch(archType) {
            case kBzip2: {
                if (UNSTARCH_extractDataWithBzip2(getInFpPtr(), 
                                                  out,
                                                  chr.c_str(), 
                                                  (const Metadata *)getArchiveMd(), 
                                                  getArchiveMdOffset(),
                                                  (const unsigned int)getArchiveHeaderFlag() ) != 0 ) {
                    throw(std::string("ERROR: backend extraction failed"));
                }            
                break;
            }
            case kGzip: {
                if (UNSTARCH_extractDataWithGzip(getInFpPtr(), 
                                                 out,
                                                 chr.c_str(), 
                                                 (const Metadata *)getArchiveMd(), 
                                                 getArchiveMdOffset(), 
                                                 (const unsigned int)getArchiveHeaderFlag()) != 0 ) {
                    throw(std::string("ERROR: backend extraction failed"));
                }
                break;
            }
            case kUndefined: {
                throw(std::string("ERROR: backend compression type is undefined"));
            }
        }
        return EXIT_SUCCESS;
    }

    int
    Starch::setupPerLineAccess()
    { 
#ifdef DEBUG
        std::fprintf(stderr, "\n--- Starch::setupPerLineAccess() ---\n");
#endif        
        if (!archMd) // read in Metadata object from JSON header
            readJSONMetadata(false, false);

        setArchiveMdIter(archMd); // set Metadata iterator position to first Metadata record
        seekCurrentInFpPosition(); // set inFp to correct position

        // set up backend decompression works
        switch (archType) {
            case kBzip2: {
                setupBzip2Works();
                break;
            }
            case kGzip: {
                setupGzipWorks();
                break;
            }
            case kUndefined: {
                throw(std::string("ERROR: backend compression type is undefined"));
            }
        }

        return EXIT_SUCCESS;
    }
}

#endif