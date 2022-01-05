#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>
#include <iostream>
#include <sstream>
#include <exception>
#include "nvEncodeAPI.h"

class NVENCException : public std::exception
{
public:
    NVENCException(const std::string& errorStr, const NVENCSTATUS errorCode)
        : m_errorString(errorStr), m_errorCode(errorCode) {}

    virtual ~NVENCException() throw() {}
    virtual const char* what() const throw() { return m_errorString.c_str(); }
    NVENCSTATUS  getErrorCode() const { return m_errorCode; }
    const std::string& getErrorString() const { return m_errorString; }
    static NVENCException makeNVENCException(const std::string& errorStr, const NVENCSTATUS errorCode,
        const std::string& functionName, const std::string& fileName, int lineNo);
private:
    std::string m_errorString;
    NVENCSTATUS m_errorCode;
};

static std::string errString[] = {
    "NV_ENC_SUCCESS",
    "NV_ENC_ERR_NO_ENCODE_DEVICE",
    "NV_ENC_ERR_UNSUPPORTED_DEVICE",
    "NV_ENC_ERR_INVALID_ENCODERDEVICE",
    "NV_ENC_ERR_INVALID_DEVICE",
    "NV_ENC_ERR_DEVICE_NOT_EXIST",
    "NV_ENC_ERR_INVALID_PTR",
    "NV_ENC_ERR_INVALID_EVENT",
    "NV_ENC_ERR_INVALID_PARAM",
    "NV_ENC_ERR_INVALID_CALL",
    "NV_ENC_ERR_OUT_OF_MEMORY",
    "NV_ENC_ERR_ENCODER_NOT_INITIALIZED",
    "NV_ENC_ERR_UNSUPPORTED_PARAM",
    "NV_ENC_ERR_LOCK_BUSY",
    "NV_ENC_ERR_NOT_ENOUGH_BUFFER",
    "NV_ENC_ERR_INVALID_VERSION",
    "NV_ENC_ERR_MAP_FAILED",
    "NV_ENC_ERR_NEED_MORE_INPUT",
    "NV_ENC_ERR_ENCODER_BUSY",
    "NV_ENC_ERR_EVENT_NOT_REGISTERD",
    "NV_ENC_ERR_GENERIC",
    "NV_ENC_ERR_INCOMPATIBLE_CLIENT_KEY",
    "NV_ENC_ERR_UNIMPLEMENTED",
    "NV_ENC_ERR_RESOURCE_REGISTER_FAILED",
    "NV_ENC_ERR_RESOURCE_NOT_REGISTERED",
    "NV_ENC_ERR_RESOURCE_NOT_MAPPED"
};

#define NVENC_THROW_ERROR( errorStr, errorCode )                                                         \
    do                                                                                                   \
    {                                                                                                    \
        throw NVENCException::makeNVENCException(errorStr, errorCode, __FUNCTION__, __FILE__, __LINE__); \
    } while (0)


#define NVENC_API_CALL( nvencAPI )                                                                                 \
    do                                                                                                             \
    {                                                                                                              \
        NVENCSTATUS errorCode = nvencAPI;                                                                          \
        if( errorCode != NV_ENC_SUCCESS)                                                                           \
        {                                                                                                          \
            std::ostringstream errorLog;                                                                           \
            if (errorCode >= 0 || errorCode < 26) {                                                                \
                errorLog << #nvencAPI << " returned error " << errorCode << " : " << errString[errorCode];         \
            } else {                                                                                               \
                errorLog << #nvencAPI << " returned error " << errorCode;                                          \
            }                                                                                                      \
            throw NVENCException::makeNVENCException(errorLog.str(), errorCode, __FUNCTION__, __FILE__, __LINE__); \
        }                                                                                                          \
    } while (0)


inline bool check(int e, int iLine, const char *szFile) {
    if (e < 0) {
        std::cout << "General error " << e << " at line " << iLine << " in file " << szFile <<std::endl;
        return false;
    }
    return true;
}

#define ck(call) check(call, __LINE__, __FILE__)
#endif // EXCEPTION_H
