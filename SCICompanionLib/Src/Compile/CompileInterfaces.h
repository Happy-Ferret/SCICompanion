#pragma once

#include "Types.h"

enum class ResolvedToken
{
    ImmediateValue,
    GlobalVariable,
    ScriptVariable,
    ScriptString,
    TempVariable,
    Parameter,
    Class,
    Instance, // Might need more...
    ExportInstance, // Instance in another script.
    ClassProperty,
    Self,
    Unknown,
};

enum ProcedureType
{
    ProcedureUnknown,
    ProcedureMain,      // Something in the main script (wIndex)
    ProcedureExternal,  // Something in another script  (wScript, wIndex)
    ProcedureLocal,     // Something in the current script (wIndex)
    ProcedureKernel,    // A kernel function (wIndex)
};

//
// Allows a source code component to declare its filename/position.
// Also supports an "end" position, optionally (by default == to start position)
//
class ISourceCodePosition
{
public:
    ISourceCodePosition() {}
    ISourceCodePosition(const ISourceCodePosition &src) = default;
    int GetLineNumber() const { return _start.Line(); }
    int GetColumnNumber() const { return _start.Column(); }
    int GetEndLineNumber() const { return _end.Line(); }
    void SetPosition(LineCol pos) { _start = pos; _end = pos; }
    void SetEndPosition(LineCol pos) { _end = pos; }
    LineCol GetPosition() const { return _start; }
    LineCol GetEndPosition() const { return _end; }
protected:
    ISourceCodePosition &operator=(const ISourceCodePosition &src) = default;

private:
    LineCol _start;
    LineCol _end;
};


class CompileContext;
class IVariableLookupContext
{
public:
    virtual ResolvedToken LookupVariableName(CompileContext &context, const std::string &str, WORD &wIndex, SpeciesIndex &dataType) const = 0;
};

// Selector/value pair.
struct species_property
{
    WORD wSelector;
    WORD wValue;
    SpeciesIndex wType; // DataTypeNone means no type was specified.
    bool fTrackRelocation;
};


enum IntegerFlags
{
    IFNone =      0x00000000,
    IFHex =       0x00000001,  // Originally represented as a hex number.
    IFNegative =  0x00000002,  // Originally represented as a -ve number. (value is already correct)
};

//
// This represents a result of compiling, which may include a line number and
// script reference.
// It can also represent an index into a textual resource.
//
class CompileResult
{
public:
    enum CompileResultType
    {
        CRT_Error,
        CRT_Warning,
        CRT_Message,
    };

    CompileResult()
    {
        _nLine = 0;
        _nCol = 0;
        _type = CRT_Message;
        _resourceType = ResourceType::None;
    }

    CompileResult(const std::string &message, CompileResultType type = CRT_Message)
    {
        _message = message;
        _nLine = 0;
        _nCol = 0;
        _type = type;
        _resourceType = ResourceType::None;
    }

    // For other resources.
    CompileResult(const std::string &message, ResourceType type, int number, int index)
    {
        _message = message;
        _nLine = number;
        _nCol = index;
        _type = CRT_Message;
        _resourceType = type;
    }

    CompileResult(const std::string &message, ScriptId script, int nLineNumber)
    {
        _message = message;
        _script = script;
        _nLine = nLineNumber;
        _nCol = 0;
        _type = CRT_Message;
        _resourceType = ResourceType::None;
    }
    CompileResult(const std::string &message, ScriptId script, int nLineNumber, int nCol, CompileResultType type)
    {
        _message = message;
        _script = script;
        _nLine = nLineNumber;
        _nCol = nCol;
        _type = type;
        _resourceType = ResourceType::None;
    }

    bool IsError() const { return (_type == CRT_Error); }
    bool IsWarning() const { return (_type == CRT_Warning); }
    ScriptId GetScript() const { return _script; }
    const std::string &GetMessage() const { return _message; }
    int GetLineNumber() const { return _nLine; }
    int GetColumn() const { return _nCol; }
    BOOL CanGotoScript() const { return !_script.IsNone(); }
    ResourceType GetResourceType() const { return _resourceType; }

private:
    std::string _message;
    ScriptId _script;
    ResourceType _resourceType;
    int _nLine;
    int _nCol;
    CompileResultType _type;
};

class ICompileLog
{
public:
    virtual void ReportResult(const CompileResult &result) = 0;
    virtual void SummarizeAndReportErrors() {}; // Optional to implement
};
