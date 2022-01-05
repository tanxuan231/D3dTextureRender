#include "exception.h"

NVENCException NVENCException::makeNVENCException(
        const std::string& errorStr, const NVENCSTATUS errorCode,
        const std::string& functionName, const std::string& fileName, int lineNo)
{
    std::ostringstream errorLog;
    errorLog << functionName << " : " << errorStr << " at " << fileName << ":" << lineNo << std::endl;
    NVENCException exception(errorLog.str(), errorCode);
    return exception;
}
